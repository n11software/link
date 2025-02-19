#include "Client.hpp"
#include "Link.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>  // Add this for TCP_NODELAY
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdexcept>
#include <sstream>
#include <fstream>  // Add this for ofstream
#include <iomanip>  // Add this for setprecision, setw, etc.
#include <zlib.h>
#include <bzlib.h>
#include <chrono>  // Add this for timing
#include <brotli/decode.h>
#include <cstring>

namespace Link {

struct RequestMetrics {
    std::chrono::microseconds dns_resolution{0};
    std::chrono::microseconds socket_creation{0};
    std::chrono::microseconds connection_time{0};
    std::chrono::microseconds ssl_handshake{0};
    std::chrono::microseconds request_send{0};
    std::chrono::microseconds waiting_time{0};
    std::chrono::microseconds response_download{0};
    std::chrono::microseconds decompression{0};
    std::chrono::microseconds total_time{0};
    std::chrono::microseconds ssl_init{0};  // Add this field
    
    void print() const {
        struct MetricEntry {
            std::string name;
            std::chrono::microseconds duration;
        };
        
        std::vector<MetricEntry> metrics = {
            {"SSL Init", ssl_init},  // Add this entry
            {"DNS Resolution", dns_resolution},
            {"Socket Creation", socket_creation},
            {"Connection Time", connection_time},
            {"SSL Handshake", ssl_handshake},
            {"Request Send", request_send},
            {"Server Processing", waiting_time},
            {"Response Download", response_download},
            {"Decompression", decompression},
            {"Total Time", total_time}
        };
        
        std::sort(metrics.begin(), metrics.end(),
            [](const MetricEntry& a, const MetricEntry& b) {
                return a.duration > b.duration;
            });
        
        std::cout << "\n=== Request Performance Metrics ===\n";
        for (const auto& metric : metrics) {
            if (metric.duration.count() > 0) {
                double ms = metric.duration.count() / 1000.0;
                std::cout << std::fixed << std::setprecision(2)
                         << std::setw(20) << std::left << metric.name 
                         << ": " << ms << "ms\n";
            }
        }
        std::cout << "================================\n";
    }
};

class Client::ClientImpl {
public:
    bool enableSSL;
    bool verifyPeer;
    int timeout;
    std::map<std::string, std::string> headers;
    SSL_CTX* ctx;
    std::string lastRequest;
    RequestMetrics metrics; // Changed from last_metrics to just metrics
    bool metricsEnabled{false};  // Add this near other boolean members

    ClientImpl(bool ssl) : enableSSL(ssl), verifyPeer(true), timeout(30), ctx(nullptr) {
        auto ssl_start = std::chrono::high_resolution_clock::now();
        
        if (enableSSL) {
            // Initialize OpenSSL only once
            static bool initialized = false;
            if (!initialized) {
                SSL_library_init();
                OpenSSL_add_ssl_algorithms();
                SSL_load_error_strings();
                initialized = true;
            }
            
            const SSL_METHOD* method = TLS_client_method();
            ctx = SSL_CTX_new(method);
            if (!ctx) {
                throw std::runtime_error("Failed to create SSL context");
            }

            // Set modern TLS configuration
            SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
            SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
            
            // Load system default CA certificates
            if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
                // If system certs fail, try a common location
                if (SSL_CTX_load_verify_locations(ctx, "/etc/ssl/cert.pem", nullptr) != 1) {
                    // macOS specific path
                    SSL_CTX_load_verify_locations(ctx, "/etc/ssl/cert.pem", "/private/etc/ssl/certs");
                }
            }
        }
        
        metrics.ssl_init = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - ssl_start);
            
        if (metricsEnabled) {
            metrics.print();
        }
    }

    ~ClientImpl() {
        if (ctx) SSL_CTX_free(ctx);
        EVP_cleanup();
    }

    std::string buildFullRequest(const std::string& method, const URL& parsedUrl, const std::string& body = "") const {
        // Build request line
        std::string request = method + " ";
        std::string path = parsedUrl.getPath().empty() ? "/" : parsedUrl.getPath();
        if (!parsedUrl.getQuery().empty()) {
            path += "?" + parsedUrl.getQuery();
        }
        request += path + " HTTP/1.1\r\n";

        // Add Host header first
        request += "Host: " + parsedUrl.getHost() + "\r\n";
        
        // Add all other headers
        for (const auto& [key, value] : headers) {
            if (key != "Host") { // Skip if we already added Host
                request += key + ": " + value + "\r\n";
            }
        }

        // Add Content-Length for non-empty body
        if (!body.empty()) {
            request += "Content-Length: " + std::to_string(body.length()) + "\r\n";
        }
        
        // End headers and add body
        request += "\r\n" + body;
        return request;
    }

    std::string decompress_gzip(const std::string& compressed) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib");
        }

        zs.next_in = (Bytef*)compressed.data();
        zs.avail_in = compressed.size();

        int ret;
        char outbuffer[32768];
        std::string decompressed;

        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = inflate(&zs, Z_NO_FLUSH);
            
            if (ret < 0) {
                inflateEnd(&zs);
                switch (ret) {
                    case Z_NEED_DICT:
                        throw std::runtime_error("Decompression error: dictionary needed");
                    case Z_DATA_ERROR:
                        throw std::runtime_error("Decompression error: corrupt data");
                    case Z_MEM_ERROR:
                        throw std::runtime_error("Decompression error: out of memory");
                    default:
                        throw std::runtime_error("Decompression error: " + std::to_string(ret));
                }
            }

            decompressed.append(outbuffer, sizeof(outbuffer) - zs.avail_out);
        } while (ret != Z_STREAM_END);

        inflateEnd(&zs);
        return decompressed;
    }

    std::string decompress_deflate(const std::string& compressed) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (inflateInit(&zs) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib");
        }

        zs.next_in = (Bytef*)compressed.data();
        zs.avail_in = compressed.size();

        int ret;
        char outbuffer[32768];
        std::string decompressed;

        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = inflate(&zs, 0);
            if (decompressed.size() < zs.total_out) {
                decompressed.append(outbuffer, zs.total_out - decompressed.size());
            }
        } while (ret == Z_OK);

        inflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            throw std::runtime_error("Error during deflate decompression");
        }

        return decompressed;
    }

    std::string decompress_bzip2(const std::string& compressed) {
        bz_stream bzs;
        memset(&bzs, 0, sizeof(bzs));

        if (BZ2_bzDecompressInit(&bzs, 0, 0) != BZ_OK) {
            throw std::runtime_error("Failed to initialize bzip2");
        }

        bzs.next_in = (char*)compressed.data();
        bzs.avail_in = compressed.size();

        int ret;
        char outbuffer[32768];
        std::string decompressed;

        do {
            bzs.next_out = outbuffer;
            bzs.avail_out = sizeof(outbuffer);

            ret = BZ2_bzDecompress(&bzs);
            if (ret != BZ_OK && ret != BZ_STREAM_END) {
                BZ2_bzDecompressEnd(&bzs);
                throw std::runtime_error("Error during bzip2 decompression");
            }

            decompressed.append(outbuffer, sizeof(outbuffer) - bzs.avail_out);
        } while (ret != BZ_STREAM_END);

        BZ2_bzDecompressEnd(&bzs);
        return decompressed;
    }

    Response sendRequest(const std::string& method, const std::string& url, const std::string& request_body = "") {
        auto total_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point{};
        metrics = RequestMetrics();
        URL parsedUrl(url);
        headers["Accept-Encoding"] = "gzip, deflate, br, bzip2";
        lastRequest = buildFullRequest(method, parsedUrl, request_body);
        
        bool useSSL = enableSSL && (parsedUrl.getScheme() == "https" || parsedUrl.getProtocol() == "https");
        int port = parsedUrl.getPort().empty() ? (useSSL ? 443 : 80) : std::stoi(parsedUrl.getPort());

        // DNS Resolution
        auto dns_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : total_start;
        struct hostent* host = gethostbyname(parsedUrl.getHost().c_str());
        if (!host) {
            throw std::runtime_error("Failed to resolve host: " + parsedUrl.getHost());
        }
        if (metricsEnabled) {
            metrics.dns_resolution = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - dns_start);
        }

        // Socket Creation
        auto socket_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : total_start;
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        if (metricsEnabled) {
            metrics.socket_creation = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - socket_start);
        }

        // Add TCP_NODELAY to disable Nagle's algorithm
        int flag = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        // Set timeout
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        // Connection
        auto connect_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : total_start;
        struct sockaddr_in server_addr = {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            throw std::runtime_error("Connection failed");
        }
        if (metricsEnabled) {
            metrics.connection_time = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - connect_start);
        }

        // SSL handshake if needed
        SSL* ssl = nullptr;
        if (useSSL && ctx) {
            auto ssl_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : total_start;
            ssl = SSL_new(ctx);
            if (!ssl) {
                close(sock);
                throw std::runtime_error("Failed to create SSL object");
            }

            SSL_set_fd(ssl, sock);
            
            // Set hostname for SNI
            SSL_set_tlsext_host_name(ssl, parsedUrl.getHost().c_str());

            // Perform handshake
            if (SSL_connect(ssl) != 1) {
                unsigned long err = ERR_get_error();
                char err_buf[256];
                ERR_error_string_n(err, err_buf, sizeof(err_buf));
                SSL_free(ssl);
                close(sock);
                throw std::runtime_error(std::string("SSL handshake failed: ") + err_buf);
            }

            // Verify certificate if enabled
            if (verifyPeer) {
                X509* cert = SSL_get_peer_certificate(ssl);
                if (!cert) {
                    SSL_free(ssl);
                    close(sock);
                    throw std::runtime_error("No certificate presented by peer");
                }
                X509_free(cert);
            }
            if (metricsEnabled) {
                metrics.ssl_handshake = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - ssl_start);
            }
        }

        // Send request
        auto send_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : total_start;
        ssize_t sent;
        if (useSSL) {
            sent = SSL_write(ssl, lastRequest.c_str(), lastRequest.length());
        } else {
            sent = send(sock, lastRequest.c_str(), lastRequest.length(), 0);
        }
        if (metricsEnabled) {
            metrics.request_send = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - send_start);
        }
        
        // Response download
        auto download_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : total_start;
        std::vector<char> buffer(32768);
        std::string response;
        response.reserve(65536);
        
        // Read initial headers
        bool headers_complete = false;
        size_t header_end = 0;
        
        while (!headers_complete) {
            int bytes_read = useSSL ? 
                SSL_read(ssl, buffer.data(), buffer.size()) : 
                recv(sock, buffer.data(), buffer.size(), 0);
                
            if (bytes_read <= 0) break;
            response.append(buffer.data(), bytes_read);
            
            if ((header_end = response.find("\r\n\r\n")) != std::string::npos) {
                headers_complete = true;
            }
        }

        // Parse headers
        std::string headers = response.substr(0, header_end);
        std::string hdrs = headers;
        std::transform(hdrs.begin(), hdrs.end(), hdrs.begin(), ::tolower);
        bool is_chunked = (hdrs.find("transfer-encoding: chunked") != std::string::npos);
        
        // Handle chunked transfer encoding
        std::string raw_body;
        if (is_chunked) {
            std::string temp_body = response.substr(header_end + 4);
            bool reading_chunks = true;
            
            while (reading_chunks) {
                // Read more data if needed
                size_t pos = 0;
                while (true) {
                    // Find and parse chunk size
                    size_t chunk_header_end = temp_body.find("\r\n", pos);
                    if (chunk_header_end == std::string::npos) {
                        // Need more data
                        int bytes_read = useSSL ? 
                            SSL_read(ssl, buffer.data(), buffer.size()) : 
                            recv(sock, buffer.data(), buffer.size(), 0);
                        
                        if (bytes_read <= 0) {
                            reading_chunks = false;
                            break;
                        }
                        temp_body.append(buffer.data(), bytes_read);
                        continue;
                    }

                    // Parse chunk size
                    std::string size_str = temp_body.substr(pos, chunk_header_end - pos);
                    // Remove any extensions
                    size_t semicolon = size_str.find(';');
                    if (semicolon != std::string::npos) {
                        size_str = size_str.substr(0, semicolon);
                    }
                    // Convert hex string to number
                    size_t chunk_size;
                    std::stringstream ss;
                    ss << std::hex << size_str;
                    ss >> chunk_size;

                    if (chunk_size == 0) {
                        reading_chunks = false;
                        break;
                    }

                    // Make sure we have the full chunk
                    size_t data_start = chunk_header_end + 2;
                    size_t data_end = data_start + chunk_size + 2; // +2 for trailing CRLF
                    
                    if (temp_body.length() < data_end) {
                        // Need more data
                        int bytes_read = useSSL ? 
                            SSL_read(ssl, buffer.data(), buffer.size()) : 
                            recv(sock, buffer.data(), buffer.size(), 0);
                        
                        if (bytes_read <= 0) {
                            reading_chunks = false;
                            break;
                        }
                        temp_body.append(buffer.data(), bytes_read);
                        continue;
                    }

                    // Extract chunk data (excluding trailing CRLF)
                    raw_body.append(temp_body.substr(data_start, chunk_size));
                    
                    // Move to next chunk
                    temp_body = temp_body.substr(data_end);
                    pos = 0;
                }
            }
        } else {
            raw_body = response.substr(header_end + 4);
        }
        if (metricsEnabled) {
            metrics.response_download = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - download_start);
        }

        // Handle content encoding
        std::string final_body = raw_body;
        // headers to lower
        size_t encoding_pos = hdrs.find("content-encoding: ");
        if (encoding_pos != std::string::npos) {
            size_t encoding_end = headers.find("\r\n", encoding_pos);
            std::string encoding = headers.substr(encoding_pos + 17, encoding_end - (encoding_pos + 17));
            // Remove all whitespace from encoding string
            encoding.erase(std::remove_if(encoding.begin(), encoding.end(), ::isspace), encoding.end());
            
            auto decompress_start = metricsEnabled ? std::chrono::high_resolution_clock::now() : total_start;
            if (encoding == "br") {
                const uint8_t* next_in = reinterpret_cast<const uint8_t*>(raw_body.data());
                size_t available_in = raw_body.size();
                std::string output;
                output.reserve(available_in * 4);  // Initial estimate

                BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
                if (!state) {
                    throw std::runtime_error("Failed to create Brotli decoder");
                }

                try {
                    uint8_t buffer[65536];
                    while (true) {
                        size_t available_out = sizeof(buffer);
                        uint8_t* next_out = buffer;

                        BrotliDecoderResult result = BrotliDecoderDecompressStream(
                            state, &available_in, &next_in,
                            &available_out, &next_out, nullptr);

                        size_t bytes_written = next_out - buffer;
                        if (bytes_written > 0) {
                            output.append(reinterpret_cast<char*>(buffer), bytes_written);
                        }

                        if (result == BROTLI_DECODER_RESULT_SUCCESS) {
                            break;
                        }
                        if (result == BROTLI_DECODER_RESULT_ERROR) {
                            throw std::runtime_error("Brotli decompression failed");
                        }
                        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
                            break;
                        }
                    }

                    final_body = std::move(output);
                } catch (...) {
                    BrotliDecoderDestroyInstance(state);
                    throw;
                }

                BrotliDecoderDestroyInstance(state);
            } else if (encoding == "gzip") {
                final_body = decompress_gzip(raw_body);
            } else if (encoding == "deflate") {
                final_body = decompress_deflate(raw_body);
            }
            if (metricsEnabled) {
                metrics.decompression = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - decompress_start);
            }
        }

        // Reconstruct full response
        response = headers + "\r\n\r\n" + final_body;

        if (ssl) SSL_free(ssl);
        close(sock);
        
        if (metricsEnabled) {
            metrics.total_time = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - total_start);
            
            metrics.waiting_time = metrics.total_time - 
                (metrics.dns_resolution + metrics.socket_creation + 
                 metrics.connection_time + metrics.ssl_handshake + 
                 metrics.request_send + metrics.response_download + 
                 metrics.decompression);
                 
            metrics.print();
        }

        return Response(response);
    }

private:
    bool read_exact(SSL* ssl, int sock, bool useSSL, char* data, size_t len) {
        size_t total = 0;
        while (total < len) {
            int n = useSSL ? 
                    SSL_read(ssl, data + total, len - total) :
                    recv(sock, data + total, len - total, 0);
            if (n <= 0) return false;
            total += n;
        }
        return true;
    }
};

Client::Client(bool enableSSL) : impl(std::make_unique<ClientImpl>(enableSSL)) {}
Client::~Client() = default;

Client& Client::setVerifyPeer(bool verify) {
    impl->verifyPeer = verify;
    return *this;
}

Client& Client::setTimeout(int seconds) {
    impl->timeout = seconds;
    return *this;
}

Client& Client::setHeader(const std::string& key, const std::string& value) {
    impl->headers[key] = value;
    return *this;
}

Client& Client::clearHeaders() {
    impl->headers.clear();
    return *this;
}

Response Client::Get(const std::string& url) {
    return impl->sendRequest("GET", url);
}

Response Client::Post(const std::string& url, const std::string& body) {
    return impl->sendRequest("POST", url, body);
}

Response Client::Put(const std::string& url, const std::string& body) {
    return impl->sendRequest("PUT", url, body);
}

Response Client::Delete(const std::string& url) {
    return impl->sendRequest("DELETE", url);
}

std::string Client::getHeadersRaw() const {
    std::string raw_headers;
    for (const auto& [key, value] : impl->headers) {
        raw_headers += key + ": " + value + "\r\n";
    }
    return raw_headers;
}

std::string Client::getLastRequestRaw() const {
    return impl->lastRequest;
}

Client& Client::enableMetrics(bool enable) {
    impl->metricsEnabled = enable;
    return *this;
}

} // namespace Link
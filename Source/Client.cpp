#include "Client.hpp"
#include "Link.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdexcept>
#include <sstream>
#include <zlib.h>
#include <bzlib.h>

namespace Link {

class Client::ClientImpl {
public:
    bool enableSSL;
    bool verifyPeer;
    int timeout;
    std::map<std::string, std::string> headers;
    SSL_CTX* ctx;
    std::string lastRequest;

    ClientImpl(bool ssl) : enableSSL(ssl), verifyPeer(true), timeout(30), ctx(nullptr) {
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
        URL parsedUrl(url);
        headers["Accept-Encoding"] = "gzip, deflate, br, bzip2";
        lastRequest = buildFullRequest(method, parsedUrl, request_body);
        
        bool useSSL = enableSSL && (parsedUrl.getScheme() == "https" || parsedUrl.getProtocol() == "https");
        int port = parsedUrl.getPort().empty() ? (useSSL ? 443 : 80) : std::stoi(parsedUrl.getPort());

        // Resolve host
        struct hostent* host = gethostbyname(parsedUrl.getHost().c_str());
        if (!host) {
            throw std::runtime_error("Failed to resolve host: " + parsedUrl.getHost());
        }

        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        // Set timeout
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        // Connect
        struct sockaddr_in server_addr = {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            throw std::runtime_error("Connection failed");
        }

        // SSL handshake if needed
        SSL* ssl = nullptr;
        if (useSSL && ctx) {
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
        }

        // Send request
        if (useSSL) {
            SSL_write(ssl, lastRequest.c_str(), lastRequest.length());
        } else {
            send(sock, lastRequest.c_str(), lastRequest.length(), 0);
        }

        // Read and parse headers first
        std::string header_block;
        char buffer[4096];
        bool headers_complete = false;
        std::map<std::string, std::string> response_headers;
        std::string response_body;
        bool is_chunked = false;
        size_t content_length = 0;
        z_stream* zs = nullptr;
        bz_stream* bzs = nullptr;
        
        while (!headers_complete) {
            int bytes_read = useSSL ? 
                SSL_read(ssl, buffer, sizeof(buffer)) : 
                recv(sock, buffer, sizeof(buffer), 0);
                
            if (bytes_read <= 0) break;
            
            header_block.append(buffer, bytes_read);
            size_t header_end = header_block.find("\r\n\r\n");
            
            if (header_end != std::string::npos) {
                // Parse headers
                std::string headers = header_block.substr(0, header_end);
                response_body = header_block.substr(header_end + 4);
                
                std::istringstream header_stream(headers);
                std::string line;
                std::getline(header_stream, line); // Status line
                while (std::getline(header_stream, line)) {
                    if (line.empty() || line == "\r") continue;
                    size_t colon = line.find(": ");
                    if (colon != std::string::npos) {
                        std::string key = line.substr(0, colon);
                        std::string value = line.substr(colon + 2);
                        // Remove trailing \r if present
                        if (!value.empty() && value.back() == '\r') {
                            value.pop_back();
                        }
                        response_headers[key] = value;
                    }
                }
                
                // Check transfer encoding and content length
                auto encoding_it = response_headers.find("Transfer-Encoding");
                is_chunked = (encoding_it != response_headers.end() && 
                            encoding_it->second.find("chunked") != std::string::npos);

                auto length_it = response_headers.find("Content-Length");
                if (length_it != response_headers.end()) {
                    content_length = std::stoul(length_it->second);
                }

                // Handle compression
                auto content_encoding = response_headers.find("Content-Encoding");
                if (content_encoding != response_headers.end()) {
                    if (content_encoding->second == "gzip" || content_encoding->second == "deflate") {
                        zs = new z_stream();
                        memset(zs, 0, sizeof(z_stream));
                        inflateInit2(zs, content_encoding->second == "gzip" ? 16 + MAX_WBITS : MAX_WBITS);
                    } else if (content_encoding->second == "bzip2") {
                        bzs = new bz_stream();
                        memset(bzs, 0, sizeof(bz_stream));
                        BZ2_bzDecompressInit(bzs, 0, 0);
                    }
                }
                
                headers_complete = true;
            }
        }

        // Read the response body
        if (is_chunked) {
            std::string chunk_buffer = response_body;
            size_t pos = 0;
            
            while (true) {
                // Read more data if needed
                if (pos >= chunk_buffer.length()) {
                    int bytes_read = useSSL ? 
                        SSL_read(ssl, buffer, sizeof(buffer)) : 
                        recv(sock, buffer, sizeof(buffer), 0);
                    if (bytes_read <= 0) break;
                    chunk_buffer.append(buffer, bytes_read);
                }

                // Process chunks
                size_t chunk_end = chunk_buffer.find("\r\n", pos);
                if (chunk_end == std::string::npos) continue;
                
                std::string chunk_size_str = chunk_buffer.substr(pos, chunk_end - pos);
                size_t chunk_size;
                std::istringstream(chunk_size_str) >> std::hex >> chunk_size;
                
                if (chunk_size == 0) break;
                
                pos = chunk_end + 2;
                size_t data_end = pos + chunk_size;
                
                // Process chunk data
                if (data_end <= chunk_buffer.length()) {
                    std::string chunk_data = chunk_buffer.substr(pos, chunk_size);
                    if (zs) {
                        // Decompress with zlib
                        char decomp_buffer[8192];
                        zs->next_in = (Bytef*)chunk_data.data();
                        zs->avail_in = chunk_data.size();
                        do {
                            zs->next_out = (Bytef*)decomp_buffer;
                            zs->avail_out = sizeof(decomp_buffer);
                            inflate(zs, Z_NO_FLUSH);
                            response_body.append(decomp_buffer, sizeof(decomp_buffer) - zs->avail_out);
                        } while (zs->avail_in > 0);
                    } else if (bzs) {
                        // Decompress with bzip2
                        char decomp_buffer[8192];
                        bzs->next_in = (char*)chunk_data.data();
                        bzs->avail_in = chunk_data.size();
                        do {
                            bzs->next_out = decomp_buffer;
                            bzs->avail_out = sizeof(decomp_buffer);
                            BZ2_bzDecompress(bzs);
                            response_body.append(decomp_buffer, sizeof(decomp_buffer) - bzs->avail_out);
                        } while (bzs->avail_in > 0);
                    } else {
                        response_body += chunk_data;
                    }
                    pos = data_end + 2; // Skip chunk CRLF
                }
            }
        } else if (content_length > 0) {
            // For non-chunked responses with Content-Length
            while (response_body.length() < content_length) {
                int bytes_read = useSSL ? 
                    SSL_read(ssl, buffer, sizeof(buffer)) : 
                    recv(sock, buffer, sizeof(buffer), 0);
                
                if (bytes_read <= 0) break;
                response_body.append(buffer, bytes_read);
            }
        } else if (!content_length) {
            // For responses without Content-Length, read until connection closes
            while (true) {
                int bytes_read = useSSL ? 
                    SSL_read(ssl, buffer, sizeof(buffer)) : 
                    recv(sock, buffer, sizeof(buffer), 0);
                
                if (bytes_read <= 0) break;
                response_body.append(buffer, bytes_read);
            }
        }

        // Handle compression if present
        if (!response_body.empty()) {
            auto content_encoding = response_headers.find("Content-Encoding");
            if (content_encoding != response_headers.end()) {
                if (content_encoding->second.find("gzip") != std::string::npos) {
                    response_body = decompress_gzip(response_body);
                } else if (content_encoding->second.find("deflate") != std::string::npos) {
                    response_body = decompress_deflate(response_body);
                } else if (content_encoding->second.find("br") != std::string::npos) {
                    // Note: Brotli support would need to be added here
                } else if (content_encoding->second.find("bzip2") != std::string::npos) {
                    response_body = decompress_bzip2(response_body);
                }
            }
        }

        // Cleanup
        if (zs) {
            inflateEnd(zs);
            delete zs;
        }
        if (bzs) {
            BZ2_bzDecompressEnd(bzs);
            delete bzs;
        }
        if (ssl) SSL_free(ssl);
        close(sock);

        // Construct final response
        std::string final_response = header_block.substr(0, header_block.find("\r\n\r\n") + 4);
        final_response += response_body;
        
        return Response(final_response);
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

} // namespace Link
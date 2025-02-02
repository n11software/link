#include "Response.hpp"
#include "Status.hpp"
#include "Filetype.hpp"
#include <sstream>
#include <cstring>  // Add this for memset
#include <zlib.h>
#include <bzlib.h>
#include <brotli/decode.h>
#include <iostream>
#include <fstream>

namespace Link {

Response::Response() : statusCode(200), sent(false) {
    headers["Content-Type"] = "text/plain";
}

Response::Response(const std::string& raw_response) : statusCode(200), sent(false) {
    parseRawResponse(raw_response);
    sent = true;  // Mark as sent after parsing
}

void Response::send(const std::string& body) {
    if (sent) return;
    this->body = body;
    this->sent = true;
}

void Response::sendFile(std::string filename) {
    if (sent) return;
    if (cachedFiles.find(filename) != cachedFiles.end()) {
        send(cachedFiles[filename]);
        return;
    }
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        status(404);
        send("File not found");
        return;
    }
    this->cachedFiles[filename] = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    status(200);
    this->setHeader("Content-Type", getMIMEType(filename));
    send(this->cachedFiles[filename]);
}

std::string Response::retrieveCache(std::string filename) {
    if (cachedFiles.find(filename) != cachedFiles.end()) {
        return cachedFiles[filename];
    }
    return "File not found";
}

void Response::status(int code) {
    this->statusCode = code;
}

void Response::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void Response::json(const std::string& jsonString) {
    headers["Content-Type"] = "application/json";
    send(jsonString);
}

void Response::redirect(const std::string& url) {
    status(302);
    setHeader("Location", url);
    send("");
}

void Response::parseRawResponse(const std::string& raw) {
    size_t pos = 0;
    
    // Parse status line first
    size_t eol = raw.find("\r\n");
    if (eol == std::string::npos) return;
    
    std::string statusLine = raw.substr(0, eol);
    if (statusLine.substr(0, 5) != "HTTP/") return;
    
    size_t statusPos = statusLine.find(' ');
    if (statusPos == std::string::npos) return;
    statusPos = statusLine.find(' ', statusPos + 1);
    if (statusPos == std::string::npos) return;
    
    statusCode = std::stoi(statusLine.substr(statusPos - 3, 3));
    
    // Parse headers
    pos = eol + 2;
    bool is_chunked = false;
    size_t content_length = 0;
    std::string content_encoding;

    // Parse all headers
    while (pos < raw.length()) {
        eol = raw.find("\r\n", pos);
        if (eol == std::string::npos) break;
        
        if (pos == eol) {
            pos = eol + 2;
            break;  // Empty line, body follows
        }
        
        std::string line = raw.substr(pos, eol - pos);
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2);  // Skip ": "
            headers[key] = value;

            if (key == "Transfer-Encoding" && value.find("chunked") != std::string::npos) {
                is_chunked = true;
            } else if (key == "Content-Length") {
                content_length = std::stoul(value);
            } else if (key == "Content-Encoding") {
                content_encoding = value;
            }
        }
        pos = eol + 2;
    }

    // Get body
    if (pos < raw.length()) {
        std::string decoded_body;
        
        if (is_chunked) {
            // Handle chunked encoding first
            std::string remaining = raw.substr(pos);
            size_t chunk_start = 0;
            
            while (chunk_start < remaining.length()) {
                size_t chunk_line_end = remaining.find("\r\n", chunk_start);
                if (chunk_line_end == std::string::npos) break;
                
                std::string chunk_size_str = remaining.substr(chunk_start, chunk_line_end - chunk_start);
                size_t chunk_size;
                std::istringstream(chunk_size_str) >> std::hex >> chunk_size;
                
                if (chunk_size == 0) break;
                
                size_t data_start = chunk_line_end + 2;
                decoded_body.append(remaining.substr(data_start, chunk_size));
                chunk_start = data_start + chunk_size + 2;
            }
        } else {
            decoded_body = raw.substr(pos);
        }

        // Handle compression if present
        if (!content_encoding.empty()) {
            try {
                if (content_encoding.find("br") != std::string::npos) {
                    decoded_body = decompress_brotli(decoded_body);
                }
                else if (content_encoding.find("gzip") != std::string::npos) {
                    decoded_body = decompress_gzip(decoded_body);
                }
                else if (content_encoding.find("deflate") != std::string::npos) {
                    decoded_body = decompress_deflate(decoded_body);
                }
                else if (content_encoding.find("bzip2") != std::string::npos) {
                    decoded_body = decompress_bzip2(decoded_body);
                }

                headers["Content-Length"] = std::to_string(decoded_body.length());
            }
            catch (const std::exception& e) {
                std::cerr << "Decompression failed: " << e.what() << std::endl;
            }
        }

        body = decoded_body;
    }
    
    sent = true;
}

std::string Response::serialize() const {
    std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + getReasonPhrase(statusCode) + "\r\n";
    
    // Create a temporary map with all headers
    std::map<std::string, std::string> temp_headers = headers;
    
    // Add Content-Length if body is not empty
    if (!body.empty()) {
        temp_headers["Content-Length"] = std::to_string(body.length());
    }

    // Add headers
    for (const auto& header : temp_headers) {
        response += header.first + ": " + header.second + "\r\n";
    }
    
    response += "\r\n" + body;
    return response;
}

std::string Response::decompress_gzip(const std::string& compressed) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib");
    }

    zs.next_in = (Bytef*)compressed.data();
    zs.avail_in = compressed.size();

    char outbuffer[32768];
    std::string decompressed;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        int ret = inflate(&zs, 0);
        if (decompressed.size() < zs.total_out) {
            decompressed.append(outbuffer, zs.total_out - decompressed.size());
        }
        if (ret == Z_STREAM_END) break;
        if (ret != Z_OK) {
            inflateEnd(&zs);
            throw std::runtime_error("Error during gzip decompression");
        }
    } while (true);

    inflateEnd(&zs);
    return decompressed;
}

std::string Response::decompress_deflate(const std::string& compressed) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK) {
        throw std::runtime_error("Failed to initialize deflate");
    }

    zs.next_in = (Bytef*)compressed.data();
    zs.avail_in = compressed.size();

    char outbuffer[32768];
    std::string decompressed;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        int ret = inflate(&zs, 0);
        if (decompressed.size() < zs.total_out) {
            decompressed.append(outbuffer, zs.total_out - decompressed.size());
        }
        if (ret == Z_STREAM_END) break;
        if (ret != Z_OK) {
            inflateEnd(&zs);
            throw std::runtime_error("Error during deflate decompression");
        }
    } while (true);

    inflateEnd(&zs);
    return decompressed;
}

std::string Response::decompress_bzip2(const std::string& compressed) {
    bz_stream bzs;
    memset(&bzs, 0, sizeof(bzs));

    if (BZ2_bzDecompressInit(&bzs, 0, 0) != BZ_OK) {
        throw std::runtime_error("Failed to initialize bzip2");
    }

    bzs.next_in = const_cast<char*>(compressed.data());
    bzs.avail_in = compressed.size();

    char outbuffer[32768];
    std::string decompressed;

    do {
        bzs.next_out = outbuffer;
        bzs.avail_out = sizeof(outbuffer);

        int ret = BZ2_bzDecompress(&bzs);
        if (ret != BZ_OK && ret != BZ_STREAM_END) {
            BZ2_bzDecompressEnd(&bzs);
            throw std::runtime_error("Error during bzip2 decompression");
        }

        decompressed.append(outbuffer, sizeof(outbuffer) - bzs.avail_out);
        if (ret == BZ_STREAM_END) break;
    } while (true);

    BZ2_bzDecompressEnd(&bzs);
    return decompressed;
}

std::string Response::decompress_brotli(const std::string& compressed) {
    const uint8_t* input = reinterpret_cast<const uint8_t*>(compressed.data());
    size_t input_size = compressed.size();
    
    // Create decoder state
    BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (!state) {
        throw std::runtime_error("Failed to create Brotli decoder");
    }

    // Prepare output buffer
    size_t available_out = compressed.size() * 4;  // Initial estimate
    std::string output;
    output.resize(available_out);
    uint8_t* next_out = reinterpret_cast<uint8_t*>(&output[0]);
    size_t total_out = 0;

    // Decompress
    BrotliDecoderResult result;
    do {
        size_t available_in = input_size;
        const uint8_t* next_in = input;

        result = BrotliDecoderDecompressStream(state,
            &available_in, &next_in,
            &available_out, &next_out,
            &total_out);

        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
            // Increase buffer size
            size_t current_size = output.size();
            output.resize(current_size * 2);
            next_out = reinterpret_cast<uint8_t*>(&output[0] + total_out);
            available_out = output.size() - total_out;
        }
    } while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT);

    BrotliDecoderDestroyInstance(state);

    if (result != BROTLI_DECODER_RESULT_SUCCESS) {
        throw std::runtime_error("Brotli decompression failed");
    }

    output.resize(total_out);
    return output;
}

} // namespace Link

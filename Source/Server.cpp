#include "Link.hpp"
#include "Status.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <regex>
#include <chrono>
#include <atomic>  // Add this for std::atomic

namespace Link {

namespace Color {
    const char* Reset   = "\033[0m";
    const char* Green   = "\033[32m";
    const char* Yellow  = "\033[33m";
    const char* Red     = "\033[31m";
    const char* Blue    = "\033[34m";
    const char* Purple  = "\033[35m";
}

struct ServerMetrics {
    std::atomic<uint64_t> total_requests;
    std::atomic<uint64_t> total_response_time;
    bool enabled;

    ServerMetrics() : total_requests(0), total_response_time(0), enabled(false) {}  // Initialize in constructor

    std::string format_duration(uint64_t microseconds) const {
        const char* color;
        if (microseconds < 100000) { // < 100ms
            color = Color::Green;
        } else if (microseconds < 500000) { // < 500ms
            color = Color::Yellow;
        } else {
            color = Color::Red;
        }
        return std::string(color) + std::to_string(microseconds / 1000.0) + "ms" + Color::Reset;
    }

    void print_stats() const {
        if (!enabled) return;
        uint64_t reqs = total_requests.load();
        if (reqs > 0) {
            double avg = total_response_time.load() / (double)reqs;
            std::cout << "Average response time: " << format_duration(avg) 
                     << " (" << reqs << " requests)" << std::endl;
        }
    }
};

struct RouteInfo {
    std::string pattern;
    std::vector<std::string> paramNames;
    std::regex regex;
    RouteCallback callback;
    
    RouteInfo(const std::string& path, RouteCallback cb) : callback(cb) {
        std::string regexPattern;
        size_t start = 0;
        size_t pos = path.find('[');
        
        while (pos != std::string::npos) {
            regexPattern += path.substr(start, pos - start);
            size_t end = path.find(']', pos);
            if (end != std::string::npos) {
                std::string paramName = path.substr(pos + 1, end - pos - 1);
                paramNames.push_back(paramName);
                regexPattern += "([^/]+)";
                start = end + 1;
            }
            pos = path.find('[', start);
        }
        
        regexPattern += path.substr(start);
        // Replace * with wildcard regex
        size_t wildcard = regexPattern.find('*');
        while (wildcard != std::string::npos) {
            regexPattern.replace(wildcard, 1, ".*");
            wildcard = regexPattern.find('*', wildcard + 2);
        }
        
        pattern = path;
        regex = std::regex(regexPattern);
    }
    
    bool matches(const std::string& path, Request& req) const {
        // Remove trailing slashes for matching
        std::string normalized_path = path;
        while (normalized_path.length() > 1 && normalized_path.back() == '/') {
            normalized_path.pop_back();
        }

        std::smatch matches;
        if (std::regex_match(normalized_path, matches, regex)) {
            // Start from 1 to skip the full match
            for (size_t i = 0; i < paramNames.size() && (i + 1) < matches.size(); ++i) {
                req.setParam(paramNames[i], matches[i + 1].str());
            }
            return true;
        }
        return false;
    }
};

struct Server::ServerImpl {
    bool metricsEnabled;
    std::string certFile;
    std::string keyFile;
    std::vector<RouteInfo> getRoutes;
    std::map<std::string, RouteCallback> postRoutes;
    std::map<std::string, RouteCallback> putRoutes;
    std::map<std::string, RouteCallback> deleteRoutes;
    std::map<int, ErrorCallback> errorHandlers;
    ErrorCallback defaultErrorHandler;
    SSL_CTX* ctx;
    int server_fd;
    ServerMetrics metrics;
    
    ServerImpl(bool enableMetrics, const std::string& cert, const std::string& key) 
        : metricsEnabled(enableMetrics)
        , certFile(cert)
        , keyFile(key)
        , ctx(nullptr)
        , server_fd(-1) {
        // Initialize metrics
        metrics.enabled = enableMetrics;

        // Initialize SSL
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        
        // Create SSL context with modern TLS
        ctx = SSL_CTX_new(TLS_server_method());
        if (!ctx) {
            throw std::runtime_error("Failed to create SSL context");
        }

        // Set minimum TLS version to 1.2
        SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
        
        // Set cipher list to secure defaults
        if (!SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5:!RC4")) {
            throw std::runtime_error("Failed to set cipher list");
        }

        // Set SSL options
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
        
        // Load certificate and private key
        if (SSL_CTX_use_certificate_file(ctx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
            throw std::runtime_error("Failed to load certificate from " + certFile);
        }
        
        if (SSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
            throw std::runtime_error("Failed to load private key from " + keyFile);
        }
        
        if (!SSL_CTX_check_private_key(ctx)) {
            throw std::runtime_error("Private key does not match certificate");
        }

        // Set up a basic default error handler
        defaultErrorHandler = [](const Request& req, Response& res, int code) {
            res.status(code);
            res.send("Error " + std::to_string(code) + ": " + getReasonPhrase(code));
        };
    }
    
    ~ServerImpl() {
        if (ctx) SSL_CTX_free(ctx);
        if (server_fd >= 0) close(server_fd);
        EVP_cleanup();
    }

    void handleError(const Request& req, Response& res, int code) {
        auto it = errorHandlers.find(code);
        if (it != errorHandlers.end()) {
            it->second(req, res, code);
        } else {
            defaultErrorHandler(req, res, code);
        }
    }

    void handle_request(const std::string& request_str, SSL* ssl, int client_fd, bool is_https) {
        auto start = std::chrono::high_resolution_clock::now();

        try {
            // Parse the first line manually to verify correct method/path parsing
            size_t first_line_end = request_str.find("\r\n");
            if (first_line_end == std::string::npos) return;

            std::string first_line = request_str.substr(0, first_line_end);
            size_t method_end = first_line.find(' ');
            if (method_end == std::string::npos) return;

            std::string method = first_line.substr(0, method_end);
            size_t path_start = first_line.find_first_not_of(' ', method_end);
            size_t path_end = first_line.find(' ', path_start);
            if (path_start == std::string::npos || path_end == std::string::npos) return;

            std::string path = first_line.substr(path_start, path_end - path_start);
            
            // Now create the request object
            Request req(request_str, "", is_https ? "https" : "http");
            Response res;

            bool routeFound = false;
            
            // Match against HTTP method using our parsed values
            if (method == "GET") {
                for (const auto& route : getRoutes) {
                    if (route.matches(path, req)) {
                        route.callback(req, res);
                        routeFound = true;
                        break;
                    }
                }
            } else if (method == "POST" && postRoutes.find(path) != postRoutes.end()) {
                postRoutes[path](req, res);
                routeFound = true;
            } else if (method == "PUT" && putRoutes.find(path) != putRoutes.end()) {
                putRoutes[path](req, res);
                routeFound = true;
            } else if (method == "DELETE" && deleteRoutes.find(path) != deleteRoutes.end()) {
                deleteRoutes[path](req, res);
                routeFound = true;
            }

            if (!routeFound) {
                handleError(req, res, 404);
            }

            std::string response = res.serialize();
            if (is_https) {
                SSL_write(ssl, response.c_str(), response.length());
            } else {
                send(client_fd, response.c_str(), response.length(), 0);
            }

            if (metrics.enabled) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                metrics.total_requests++;
                metrics.total_response_time += duration.count();
                std::cout << Color::Purple << "[" << req.getIP() << "] " 
                         << Color::Blue << method << " " << path 
                         << Color::Yellow << " > " 
                         << metrics.format_duration(duration.count()) 
                         << Color::Reset << std::endl;

            }

        } catch (const std::exception& e) {
            std::cerr << Color::Red << "Error handling request: " << e.what() << Color::Reset << std::endl;
        }
    }
};

Server::Server(bool enableMetrics, const std::string& certFile, const std::string& keyFile)
    : impl(std::make_unique<ServerImpl>(enableMetrics, certFile, keyFile)) {}

Server::~Server() = default;

void Server::Get(const std::string& path, RouteCallback callback) {
    impl->getRoutes.emplace_back(path, callback);
}

void Server::Post(const std::string& path, RouteCallback callback) {
    impl->postRoutes[path] = callback;
}

void Server::Put(const std::string& path, RouteCallback callback) {
    impl->putRoutes[path] = callback;
}

void Server::Delete(const std::string& path, RouteCallback callback) {
    impl->deleteRoutes[path] = callback;
}

void Server::OnError(int statusCode, ErrorCallback callback) {
    impl->errorHandlers[statusCode] = callback;
}

void Server::OnError(ErrorCallback callback) {
    impl->defaultErrorHandler = callback;
}

void Server::Listen(int port) {
    // Initialize SSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    impl->ctx = SSL_CTX_new(TLS_server_method());
    
    if (!impl->ctx) {
        throw std::runtime_error("Failed to create SSL context");
    }

    // Set minimum TLS version to 1.2
    SSL_CTX_set_min_proto_version(impl->ctx, TLS1_2_VERSION);
    
    // Set cipher list to secure defaults
    if (!SSL_CTX_set_cipher_list(impl->ctx, "HIGH:!aNULL:!MD5:!RC4")) {
        throw std::runtime_error("Failed to set cipher list");
    }

    // Set SSL options
    SSL_CTX_set_options(impl->ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
    
    // Load certificate and private key
    if (SSL_CTX_use_certificate_file(impl->ctx, impl->certFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error("Failed to load certificate from " + impl->certFile);
    }
    
    if (SSL_CTX_use_PrivateKey_file(impl->ctx, impl->keyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error("Failed to load private key from " + impl->keyFile);
    }
    
    if (!SSL_CTX_check_private_key(impl->ctx)) {
        throw std::runtime_error("Private key does not match certificate");
    }

    // Set up socket
    impl->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (impl->server_fd < 0) {
        throw std::runtime_error("Socket creation failed");
    }

    int opt = 1;
    setsockopt(impl->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(impl->server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    if (listen(impl->server_fd, SOMAXCONN) < 0) {
        throw std::runtime_error("Listen failed");
    }


    // Server loop
    while (true) {
        int client_fd = accept(impl->server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;

        // Create SSL object if needed
        SSL* ssl = SSL_new(impl->ctx);
        if (!ssl) {
            close(client_fd);
            continue;
        }
        if (SSL_set_fd(ssl, client_fd) != 1) {
            SSL_free(ssl);
            close(client_fd);
            continue;
        }
        int ret = SSL_accept(ssl);
        bool is_https = (ret > 0);

        std::thread([this, client_fd, ssl, is_https]() {
            auto start = std::chrono::high_resolution_clock::now();
            
            // Get client IP
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            getpeername(client_fd, (struct sockaddr*)&addr, &addr_len);
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
            std::string client_ip(ip_str);
            
            // Read headers first
            std::vector<char> buffer(8192);
            std::string request_str;
            size_t total_bytes = 0;
            size_t content_length = 0;
            bool headers_complete = false;
            
            while (!headers_complete) {
                int bytes_read = is_https ? 
                    SSL_read(ssl, buffer.data(), buffer.size()) :
                    recv(client_fd, buffer.data(), buffer.size(), 0);
                    
                if (bytes_read <= 0) break;
                
                request_str.append(buffer.data(), bytes_read);
                total_bytes += bytes_read;
                
                // Check if we have complete headers
                size_t header_end = request_str.find("\r\n\r\n");
                if (header_end != std::string::npos) {
                    headers_complete = true;
                    
                    // Parse Content-Length
                    size_t cl_pos = request_str.find("Content-Length: ");
                    if (cl_pos != std::string::npos) {
                        size_t cl_end = request_str.find("\r\n", cl_pos);
                        if (cl_end != std::string::npos) {
                            std::string cl_str = request_str.substr(cl_pos + 16, cl_end - (cl_pos + 16));
                            content_length = std::stoul(cl_str);
                        }
                    }
                }
            }
            
            // If we have a content length, keep reading until we get all the data
            if (content_length > 0) {
                size_t body_received = request_str.length() - request_str.find("\r\n\r\n") - 4;
                
                while (body_received < content_length) {
                    int bytes_read = is_https ? 
                        SSL_read(ssl, buffer.data(), buffer.size()) :
                        recv(client_fd, buffer.data(), buffer.size(), 0);
                        
                    if (bytes_read <= 0) break;
                    
                    request_str.append(buffer.data(), bytes_read);
                    body_received += bytes_read;
                }
            }

            if (!request_str.empty()) {
                // Process request
                Request req(request_str, client_ip, is_https ? "https" : "http");
                Response res;

                // Route handling
                bool routeFound = false;
                
                if (req.getMethod() == "GET") {
                    for (const auto& route : impl->getRoutes) {
                        if (route.matches(req.getURL().getPath(), req)) {
                            route.callback(req, res);
                            routeFound = true;
                            break;
                        }
                    }
                } else if (req.getMethod() == "POST") {
                    auto it = impl->postRoutes.find(req.getURL().getPath());
                    if (it != impl->postRoutes.end()) {
                        it->second(req, res);
                        routeFound = true;
                    }
                } else if (req.getMethod() == "PUT") {
                    auto it = impl->putRoutes.find(req.getURL().getPath());
                    if (it != impl->putRoutes.end()) {
                        it->second(req, res);
                        routeFound = true;
                    }
                } else if (req.getMethod() == "DELETE") {
                    auto it = impl->deleteRoutes.find(req.getURL().getPath());
                    if (it != impl->deleteRoutes.end()) {
                        it->second(req, res);
                        routeFound = true;
                    }
                }

                if (!routeFound) {
                    impl->handleError(req, res, 404);
                }

                std::string response = res.serialize();
                if (is_https) {
                    SSL_write(ssl, response.c_str(), response.length());
                } else {
                    send(client_fd, response.c_str(), response.length(), 0);
                }

                if (impl->metrics.enabled) {
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                    impl->metrics.total_requests++;
                    impl->metrics.total_response_time += duration.count();
                    std::cout << Color::Purple << "[" + req.getIP() + "] " + Color::Blue + req.getMethod() + " " + req.getURL().getPath() +
                        Color::Yellow << " > " << impl->metrics.format_duration(duration.count()) << Color::Reset << std::endl;
                }
            }
            if (is_https) SSL_free(ssl);
            close(client_fd);
        }).detach();
    }
}

} // namespace Link

#include <Link.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <algorithm>
#include <cctype>
#include <string>

Link::Client::Client(Link::Request* request) {
    Status = 0;
    this->request = request;
    if (this->request->GetProtocol() != "https" && this->request->GetProtocol() != "http") {
        std::cout << "Invalid protocol: " << this->request->GetProtocol() << std::endl;
    }
}

Link::Request* Link::Client::SetRequest(Link::Request* request) {
    this->request = request;
    return this->request;
}

Link::Request* Link::Client::GetRequest() {
    return this->request;
}

Link::Response* Link::Client::GetResponse() {
    return this->response;
}

int Link::Client::Write(const void* buf, size_t count) {
    if (this->request->GetProtocol() == "https") return SSL_write((SSL*)ssl, buf, count);
    else return send(sock, buf, count, 0);
}

int Link::Client::Read(void* buf, size_t count) {
    if (this->request->GetProtocol() == "https") return SSL_read((SSL*)ssl, buf, count);
    else return recv(sock, buf, count, 0);
}

bool Link::Client::getChunkSize(int& remaining, std::string& body) {
    std::string chunkSizeStr = "";
    char buffer[1];
    while (chunkSizeStr.find("\r\n") == std::string::npos) {
        int bytes = Read(buffer, 1);
        switch(buffer[0]) {
            case '\r':
            case '\n':
                if (chunkSizeStr == "" || chunkSizeStr == "\r" || chunkSizeStr == "\n") {
                    chunkSizeStr = "";
                    // body += buffer[0];
                    continue;
                }
            case 'a'...'f':
            case 'A'...'F':
            case '0'...'9':
                chunkSizeStr += buffer[0];
                break;
            default:
                body += buffer[0];
                break;
        }
    }
    remaining = std::stoi(chunkSizeStr, 0, 16);
    if (remaining == 0) return true;
    return false;
}

Link::Response* Link::Client::Send() {
    Status = 0;
    SSL_CTX* ctx = NULL;
    if (this->request->GetProtocol() == "https") {
        ctx = SSL_CTX_new(SSLv23_client_method());
    }
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    if (this->request->GetPort()==0) addr.sin_port = htons(this->request->GetProtocol()=="https"?443:80); // TODO: Parse port from URL
    else addr.sin_port = htons(this->request->GetPort());
    struct hostent* ipv4 = gethostbyname(this->request->GetDomain().c_str());
    addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)ipv4->h_addr_list[0]));
    
    socklen_t socklen = sizeof(addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr*)&addr, socklen) < 0) {
        Status = 1;
        return NULL;
    }

    if (this->request->GetProtocol() == "https") {
        SSL_library_init();
        SSLeay_add_ssl_algorithms();
        SSL_load_error_strings();
        // SSL_CTX_set_cipher_list(ctx, "TLS_AES_256_GCM_SHA384");
        SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
        ssl = SSL_new(ctx);
        int newsock = SSL_get_fd(ssl);
        SSL_set_fd(ssl, sock);
        SSL_set_tlsext_host_name(ssl, this->request->GetDomain().c_str());
        int error = SSL_connect(ssl);
        if (error < 0) {
            Status = 2;
            return NULL;
        }
    }

    std::string r = this->request->GetRawHeaders();
    r += "\r\n";
    r += this->request->GetBody();
    
    int status = Write(r.c_str(), strlen(r.c_str()));
    if (status < 0) {
        Status = 3;
        return NULL;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    int remaining = 0;
    char buffer[1024];
    std::string response;

    while (response.find("\r\n\r\n") == std::string::npos) {
        int bytes = Read(buffer, 1);
        if (bytes > 0) response += std::string(buffer, bytes);
    }

    std::string body, headers;

    std::string lower = response;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);


    if (lower.find("transfer-encoding: chunked") != std::string::npos) {
        /*
         * Finally figured out how to do this
         * This should not be a problem anymore
         * - FiRe
         */
        // Check for the first chunk size
        headers = response.substr(0, response.find("\r\n\r\n"));
        body = response.substr(response.find("\r\n\r\n") + 4, response.find("\r\n0\r\n\r\n") - response.find("\r\n\r\n") - 4);
        if (body.substr(0, body.find("\r\n")).length() > 0) {
            // We have a chunk size
            /*
             * This check is no longer needed, but I'm leaving it here for now
             * - FiRe
             */
            remaining = std::stoi(body.substr(0, body.find("\r\n")), 0, 16);
        } else {
            // Need to find the next chunk size
            getChunkSize(remaining, body);
        }
        while (remaining > 0) {
            int bytes = Read(buffer, remaining>1024?1024:remaining);
            if (bytes > 0) {
                body += std::string(buffer, bytes);
                remaining -= bytes;
            }
        }
        
        while (!getChunkSize(remaining, body)) {
            while (remaining > 0) {
                int bytes = Read(buffer, remaining>1024?1024:remaining);
                if (bytes > 0) {
                    body += std::string(buffer, bytes);
                    remaining -= bytes;
                }
            }
        }
    } else if (lower.find("content-length: ") != std::string::npos) {
        remaining = std::stoi(lower.substr(lower.find("content-length: ") + 16, lower.find("\r\n", lower.find("content-length: ") + 16) - lower.find("content-length: ") - 16));
        remaining-=response.substr(response.find("\r\n\r\n") + 4).length();
        if (remaining > 0) {
            while (remaining > 0) {
                int bytes = Read(buffer, 1024);
                if (bytes > 0) {
                    response += std::string(buffer, bytes);
                    remaining -= bytes;
                }
            }
        }
        body = response.substr(response.find("\r\n\r\n") + 4);
        headers = response.substr(0, response.find("\r\n\r\n"));
    } else headers = response;
    Response* res = new Response(headers, body);

    if (this->request->GetProtocol() == "https") {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    close(sock);
    return res;
}
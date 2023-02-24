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
    this->request = request;
    if (this->request->GetProtocol() != "https" && this->request->GetProtocol() != "http") {
        std::cout << "Invalid protocol: " << this->request->GetProtocol() << std::endl;
    }
    this->port = 0;
}

int Link::Client::Write(const void* buf, size_t count) {
    if (this->request->GetProtocol() == "https") return SSL_write((SSL*)ssl, buf, count);
    else return write(sock, buf, count);
}

int Link::Client::Read(void* buf, size_t count) {
    if (this->request->GetProtocol() == "https") return SSL_read((SSL*)ssl, buf, count);
    else return read(sock, buf, count);
}

const SSL_METHOD* method = SSLv23_client_method();
SSL_CTX* ctx = SSL_CTX_new(method);

Link::Response* Link::Client::Send() {
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    if (this->port==0) addr.sin_port = htons(this->request->GetProtocol()=="https"?443:80);
    else addr.sin_port = htons(this->port);
    struct hostent* ipv4 = gethostbyname(this->request->GetDomain().c_str());
    addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)ipv4->h_addr_list[0]));
    std::cout << inet_ntoa(*(struct in_addr*)ipv4->h_addr_list[0]) << std::endl;
    
    socklen_t socklen = sizeof(addr);
    if (connect(sock, (struct sockaddr*)&addr, socklen) < 0) std::cout << "Connection failed" << std::endl;

    if (this->request->GetProtocol() == "https") {
        SSL_library_init();
        SSLeay_add_ssl_algorithms();
        SSL_load_error_strings();
        SSL_CTX_set_cipher_list(ctx, "TLS_AES_256_GCM_SHA384");
        SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
        ssl = SSL_new(ctx);
        int newsock = SSL_get_fd(ssl);
        SSL_set_fd(ssl, sock);
        SSL_set_tlsext_host_name(ssl, this->request->GetDomain().c_str());
        int error = SSL_connect(ssl);
        if (error < 0) std::cout << "SSL connection failed" << std::endl;
        else std::cout << "SSL connection established" << std::endl;
    }

    std::string request = this->request->GetMethod() + " " + this->request->GetPath() + " HTTP/1.1\r\n";
    request += "Host: " + this->request->GetDomain() + "\r\n";
    request += "User-Agent: Link/2.0.0\r\n\r\n";
    
    int status = Write(request.c_str(), strlen(request.c_str()));
    if (status < 0) std::cout << "Write failed: " << status << std::endl;

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    int remaining = 0;
    char buffer[1024];
    std::string response;

    while (response.find("\r\n\r\n") == std::string::npos) {
        int bytes = Read(buffer, 1024);
        if (bytes > 0) response += std::string(buffer, bytes);
    }

    std::string body, headers;

    std::string lower = response;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower.find("transfer-encoding: chunked") != std::string::npos) {
        // Stupid way to do this but most efficient and reliable

        while (response.find("\r\n0\r\n\r\n") == std::string::npos) {
            int bytes = Read(buffer, 1024);
            if (bytes > 0) response += std::string(buffer, bytes);
        }

        headers = response.substr(0, response.find("\r\n\r\n"));
        body = response.substr(response.find("\r\n\r\n") + 4, response.find("\r\n0\r\n\r\n") - response.find("\r\n\r\n") - 4);
    } else {
        remaining = std::stoi(response.substr(response.find("Content-Length: ") + 16, response.find("\r\n", response.find("Content-Length: ") + 16) - response.find("Content-Length: ") - 16));
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
    }
    Response* res = new Response(headers, body);

    if (this->request->GetProtocol() == "https") {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    close(sock);
    return res;
}
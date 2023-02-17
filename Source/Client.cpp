#include <Link.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>

Link::Client::Client(Link::Request* request) {
    this->request = request;
    if (this->request->GetProtocol() != "https" && this->request->GetProtocol() != "http") {
        std::cout << "Invalid protocol: " << this->request->GetProtocol() << std::endl;
    }
}

void Link::Client::Send() {
    SSL* ssl;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    if (this->port==0) addr.sin_port = htons(this->request->GetProtocol()=="https"?443:80);
    else addr.sin_port = htons(this->port);
    struct hostent* ipv4 = gethostbyname(this->request->GetDomain().c_str());
    addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)ipv4->h_addr_list[0]));
    std::cout << inet_ntoa(*(struct in_addr*)ipv4->h_addr_list[0]) << std::endl;
    
    socklen_t socklen = sizeof(addr);
    if (connect(sock, (struct sockaddr*)&addr, socklen) < 0) std::cout << "Connection failed" << std::endl;
    
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD* method = SSLv23_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    ssl = SSL_new(ctx);
    int newsock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);

    std::string request = this->request->GetMethod() + " " + this->request->GetPath() + " HTTP/1.1\r\n";
    request += "Host: " + this->request->GetDomain() + "\r\n";
    request += "User-Agent: Link/1.0\r\n\r\n";

    if (SSL_write(ssl, request.c_str(), request.length()) < 0) std::cout << "Write failed" << std::endl;

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    int remaining = 0;
    char buffer[1024];
    std::string response;

    while (response.find("\r\n\r\n") == std::string::npos) {
        int bytes = SSL_read(ssl, buffer, 1024);
        if (bytes > 0) response += std::string(buffer, bytes);
    }

    if (response.find("Transfer-Encoding: chunked") != std::string::npos) {
        std::string chunk;
        while (true) {
            // remove chunk size from response
            response = response.substr(response.find("\r\n"));

            int bytes = SSL_read(ssl, buffer, 1024);
            if (std::string(buffer).find("0\r\n\r\n") != std::string::npos) break;
            if (bytes > 0) response += std::string(buffer, bytes);
        }
    } else {
        remaining = std::stoi(response.substr(response.find("Content-Length: ") + 16, response.find("\r\n", response.find("Content-Length: ") + 16) - response.find("Content-Length: ") - 16));

        while (remaining > 0) {
            int bytes = SSL_read(ssl, buffer, 1024);
            if (bytes > 0) {
                response += std::string(buffer, bytes);
                remaining -= bytes;
            }
        }
    }

    std::ofstream file;
    file.open("response.html");
    file << response;
    file.close();

    std::cout << response << std::endl;


    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);
}
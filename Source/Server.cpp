#include <Link.hpp>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <fcntl.h>
#include <functional>
#include <fstream>
#include <filesystem>

Link::Server::Server() {
    this->port = 0;
}

Link::Server::Server(int port) {
    this->port = port;
}

Link::Server* Link::Server::SetPort(int port) {
    this->port = port;
    return this;
}

Link::Server* Link::Server::EnableMultiThreaded() {
    this->multiThreaded = true;
    return this;
}

Link::Server* Link::Server::DisableMultiThreaded() {
    this->multiThreaded = false;
    return this;
}

Link::Server* Link::Server::EnableSSL(std::string certPath, std::string keyPath) {
    this->sslEnabled = true;
    this->certPath = certPath;
    this->keyPath = keyPath;
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (ctx == NULL) {
        std::cout << "SSL_CTX_new failed" << std::endl;
        return this;
    }
    SSL_CTX_set_ecdh_auto(ctx, 1);
    if (SSL_CTX_use_certificate_file(ctx, this->certPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cout << "SSL_CTX_use_certificate_file failed" << std::endl;
        return this;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, this->keyPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cout << "SSL_CTX_use_PrivateKey_file failed" << std::endl;
        return this;
    }
    this->ctx = ctx;
    return this;
}

bool Link::Server::IsRunning() {
    return this->running;
}

bool Link::Server::IsMultiThreaded() {
    return this->multiThreaded;
}

bool Link::Server::IsSSL() {
    return this->sslEnabled;
}

int Link::Server::GetPort() {
    return this->port;
}

struct HandlerArgs {
    Link::Server* server;
    int clientSock;
    SSL* ssl;
    bool sslEnabled;
};

void HandlerWrapper(void* raw) {
    HandlerArgs* args = (HandlerArgs*) raw;
    if (args->sslEnabled) {
        Link::Thread thread(args->server, args->ssl, args->sslEnabled);
        thread.Run();
    } else {
        Link::Thread thread(args->server, args->clientSock, args->sslEnabled);
        thread.Run();
    }
    free(args);
}

Link::Server* Link::Server::Get(std::string path, std::function<void(Request*, Response*)> callback) {
    std::vector<std::string> key = {"GET", path};
    this->callbacks[key] = callback;
    return this;
}

Link::Server* Link::Server::Post(std::string path, std::function<void(Request*, Response*)> callback) {
    std::vector<std::string> key = {"POST", path};
    this->callbacks[key] = callback;
    return this;
}

Link::Server* Link::Server::Route(std::string method, std::string path, std::function<void(Request*, Response*)> callback) {
    std::vector<std::string> key = {method, path};
    this->callbacks[key] = callback;
    return this;
}

Link::Server* Link::Server::Error(int code, std::function<void(Request*, Response*)> callback) {
    this->errors[code] = callback;
    return this;
}

std::map<int, std::function<void(Link::Request*, Link::Response*)>> Link::Server::GetErrors() {
    return this->errors;
}

std::map<std::vector<std::string>, std::function<void(Link::Request*, Link::Response*)>> Link::Server::GetCallbacks() {
    return this->callbacks;
}

Link::Server* Link::Server::SetStaticPages(std::string path) {
    this->staticPages = path;
    return this;
}

std::string Link::Server::GetStaticPagesDirectory() {
    return this->staticPages;
}

std::vector<std::string> Link::Server::GetStaticPages() {
    if (this->staticPages == "" || !std::filesystem::exists(this->staticPages)) return std::vector<std::string>();
    std::vector<std::string> pages;
    for (const auto& entry : std::filesystem::directory_iterator(this->staticPages)) {
        pages.push_back(entry.path());
    }
    return pages;
}

Link::Server* Link::Server::Start() {
    running = true;
    if (this->port == 0) port = sslEnabled ? 443 : 80;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) return this;

    if (sock < 0) {
        perror("Socket error");
        exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(this->port);

    if (bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("Bind error");
        exit(1);
    }

    if (listen(sock, 128) < 0) {
        perror("Listen error");
        exit(1);
    }

    while (this->running) {
        struct sockaddr_in client;
        socklen_t clientLen = sizeof(client);
        int clientSock = accept(sock, (struct sockaddr*) &client, &clientLen);
        if (clientSock < 0) {
            perror("Accept error");
            exit(1);
        }
        SSL* ssl;

        bool ClientSSL = false;

        if (sslEnabled) {
            char buffer[1];
            int bytes = recv(clientSock, buffer, sizeof(buffer), MSG_PEEK);
            if (buffer[0] == '\x16') {
                ssl = SSL_new(ctx);
                SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
                if (SSL_set_fd(ssl, clientSock) <= 0) {
                    printf("ERROR: could not set SSL file descriptor\n");
                } else {
                    int ret = SSL_accept(ssl);
                    if (ret <= 0) {
                        // int err = SSL_get_error(ssl, ret);
                        // printf("ERROR: could not accept SSL connection: %d\n", err);
                    } else {
                        ClientSSL = true;
                    }
                }
            } else {
                close(clientSock);
                continue;
            }
        }

        if (this->multiThreaded) {
            pthread_t thread;
            HandlerArgs* args = (HandlerArgs*) malloc(sizeof(HandlerArgs));
            args->server = this;
            args->clientSock = clientSock;
            args->ssl = ssl;
            args->sslEnabled = ClientSSL;
            pthread_create(&thread, NULL, (void* (*)(void*)) HandlerWrapper, (void*) args);
        } else {
            if (ClientSSL) {
                Link::Thread thread(this, ssl, ClientSSL);
                thread.Run();
            } else {
                Link::Thread thread(this, clientSock, ClientSSL);
                thread.Run();
            }
        }

        if (sslEnabled&&ClientSSL) {
            SSL_clear(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ClientSSL = false;
        }
        close(clientSock);
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);
    EVP_cleanup();

    return this;
}
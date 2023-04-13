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
    this->multiThreaded = false;
    this->sslEnabled = false;
    this->debugging = false;
    this->Status = 0;
}

Link::Server* Link::Server::EnableDebugging() {
    this->debugging = true;
    return this;
}

bool Link::Server::IsDebugging() {
    return this->debugging;
}

Link::Server::Server(int port) {
    this->port = port;
    this->multiThreaded = false;
    this->sslEnabled = false;
    this->debugging = false;
    this->Status = 0;
}

Link::Server* Link::Server::SetPort(int port) {
    this->port = port;
    return this;
}

Link::Server* Link::Server::EnableMultiThreading() {
    this->multiThreaded = true;
    return this;
}

Link::Server* Link::Server::Stop() {
    this->running = false;
    return this;
}

Link::Server* Link::Server::Use(std::function<void(Link::Request*, Link::Response*, Link::Server*)> middleware) {
    this->middlewares.push_back(middleware);
    return this;
}

std::vector<std::function<void(Link::Request*, Link::Response*, Link::Server*)>> Link::Server::GetMiddlewares() {
    return this->middlewares;
}

Link::Server* Link::Server::DisableMultiThreading() {
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
    Link::Thread* thread;
};

void HandlerWrapper(void* raw) {
    HandlerArgs* args = (HandlerArgs*) raw;
    Link::Thread* thread = args->thread;
    thread->Run();
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
    for (std::filesystem::recursive_directory_iterator i(staticPages), end; i != end; ++i) 
    if (!std::filesystem::is_directory(i->path())) {
      pages.push_back(i->path().parent_path().string()+'/'+i->path().filename().string());
    }
    return pages;
}

Link::Server* Link::Server::SetStartMessage(std::string msg) {
    this->startMessage = msg;
    return this;
}

Link::Server* Link::Server::Start() {
    running = true;
    if (this->port == 0) port = sslEnabled ? 443 : 80;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) return this;

    if (sock < 0) {
        Status = 1;
        return this;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(this->port);

    if (bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
        Status = 2;
        return this;
    }

    if (listen(sock, 128) < 0) {
        Status = 3;
        return this;
    }

    if (startMessage.length() > 0) std::cout << startMessage << std::endl;

    while (this->running) {
        struct sockaddr_in client;
        socklen_t clientLen = sizeof(client);
        int clientSock = accept(sock, (struct sockaddr*) &client, &clientLen);
        if (clientSock < 0) {
            Status = 4;
            return this;
        }
        SSL* ssl;

        bool ClientSSL = false;

        if (sslEnabled) {
            char buffer[1];
            int bytes = recv(clientSock, buffer, sizeof(buffer), MSG_PEEK);
            if (buffer[0] == '\x16') {
                ssl = (SSL*) malloc(sizeof(SSL*));
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
            std::string ip = inet_ntoa(client.sin_addr);
            Link::Thread t;
            if (ClientSSL&&sslEnabled) {
                t = Link::Thread(this, ssl, ClientSSL);
                t.SetIP(ip);
            } else {
                t = Link::Thread(this, clientSock, ClientSSL);
                t.SetIP(ip);
            }

            HandlerArgs* args = new HandlerArgs();
            args->thread = &t;
            pthread_create(&thread, NULL, (void* (*)(void*)) HandlerWrapper, (void*) args);
            pthread_join(thread, NULL);
        } else {
            std::string ip = inet_ntoa(client.sin_addr);
            if (ClientSSL) {
                Link::Thread thread(this, ssl, ClientSSL);
                thread.SetIP(ip);
                thread.Run();
            } else {
                Link::Thread thread(this, clientSock, ClientSSL);
                thread.SetIP(ip);
                thread.Run();
            }
        }
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);
    EVP_cleanup();

    return this;
}
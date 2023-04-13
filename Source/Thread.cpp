#include <Link.hpp>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>

int Link::Thread::Write(const void* buf, size_t count) {
    if (this->sslEnabled) return SSL_write((SSL*)ssl, buf, count);
    else {
        int sent = 0;
        while (sent < count) {
            int toSend = count - sent;
            if (toSend > 1024) toSend = 1024;
            int res = send(sock, (char*)buf + sent, toSend, MSG_NOSIGNAL);
            if (res < 0) return res;
            sent += res;
        }
        return sent;
    }
}

int Link::Thread::Read(void* buf, size_t count) {
    try {
        if (this->sslEnabled) return SSL_read((SSL*)ssl, buf, count);
        else return recv(sock, buf, count, 0);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
}

Link::Thread::Thread(Server* server, int sock, bool sslEnabled) {
    this->server = server;
    this->sock = sock;
    this->sslEnabled = sslEnabled;
}

Link::Thread::Thread(Server* server, SSL* ssl, bool sslEnabled) {
    this->server = server;
    this->ssl = ssl;
    this->sslEnabled = sslEnabled;
}

std::vector<std::string> split(std::string data, std::string delimiter) {
    std::vector<std::string> result;
    size_t pos = 0;
    std::string token;
    while ((pos = data.find(delimiter)) != std::string::npos) {
        token = data.substr(0, pos);
        if (token != "") result.push_back(token);
        data.erase(0, pos + delimiter.length());
    }
    if (data != "") result.push_back(data);
    return result;
}

void Link::Thread::SetIP(std::string ip) {
    this->ip = ip;
}

void Link::Thread::Run() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    char buffer[1];
    std::string request = "";
    while (Read(buffer, 1) > 0) {
        request += buffer[0];
        if (request.find("\r\n\r\n") != std::string::npos) break;
    }

    if (request == "") {
        if (server->IsMultiThreaded()) pthread_exit(NULL);
        return;
    }

    std::string lower;
    std::transform(request.begin(), request.end(), std::back_inserter(lower), ::tolower);
    std::string headers = request.substr(0, request.find("\r\n\r\n"));

    Link::Request* req = new Link::Request(headers, "");
    req->SetProtocol(this->sslEnabled ? "https" : "http");

    std::string body;
    if (req->GetHeader("Content-Length") != "") {
        for (int x=0;x<std::stoi(req->GetHeader("Content-Length"));x++) {
            Read(buffer, 1);
            body += buffer[0];
        }
    }

    req->SetBody(body);
    req->SetIP(this->ip);

    Link::Response* response = new Link::Response();
    response->SetHeader("Content-Type", "text/html");
    if (req->GetMethod() == "" || req->GetPath() == "") {
        response->SetStatus(400);
        response->SetBody("400 Bad Request");
        std::string res = response->GetVersion() + " " + std::to_string(response->GetStatus()) + " " + Link::Status(response->GetStatus()) + "\r\n";
        res = response->GetHeadersRaw() + "\r\n" + response->GetBody();
        Write(res.c_str(), res.length());
        if (server->IsMultiThreaded()) pthread_exit(NULL);
        else return;
    }

    std::vector<std::string> staticPages = server->GetStaticPages();

    bool found = false;

    for (auto middleware : server->GetMiddlewares()) {
        middleware(req, response, server);
    }

    if (response->isClosed()) found = true;

    if (!found) for (auto route : server->GetCallbacks()) {
        if (route.first[0] == req->GetMethod()) {
            std::string routePath = route.first[1];
            std::string reqPath = req->GetPath();
            if (routePath == reqPath) {
                found = true;
                route.second(req, response);
                if (response->isClosed()) found = true;
                break;
            }
            std::vector<std::string> routePathSplit = split(routePath, "/");
            std::vector<std::string> reqPathSplit = split(reqPath, "/");
            if (routePathSplit.size() == reqPathSplit.size()) {
                bool match = true;
                for (int x=0;x<routePathSplit.size();x++) {
                    if (routePathSplit[x] != reqPathSplit[x]) {
                        if (routePathSplit[x][0] == ':') {
                            req->SetParam(routePathSplit[x].substr(1, routePathSplit[x].length()), reqPathSplit[x]);
                        } else {
                            match = false;
                            break;
                        }
                    }
                }
                if (match) {
                    found = true;
                    try {
                        route.second(req, response);
                        if (response->isClosed()) found = true;
                    } catch (std::exception e) {
                        perror("Link: Error while executing route!");
                    }
                    break;
                }
            }
        }
    }

    if (!found) {
        for (int i=0;i<staticPages.size();i++) {
            std::string route = staticPages[i].substr(server->GetStaticPagesDirectory().length());
            if (route[0] != '/') route = "/" + route;
            if (req->GetPath() == route) {
                std::string path = staticPages[i];
                std::ifstream file(path);
                if (file.is_open()) {
                    found = true;
                    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    response->SetBody(content);
                    response->SetStatus(200);
                    response->SetHeader("Content-Type", Link::GetMIMEType(path));
                    if (response->GetHeader("Content-Type").substr(0,4) == "font" || req->GetPath().substr(0, 5) == "/font") {
                        response->SetHeader("Access-Control-Allow-Origin", "*")->SetHeader("Cache-Control", "public, max-age=31536000")->SetHeader("Accept-Ranges", "bytes");
                    }
                }
                break;
            }
        }
    }

    if (!found) {
        response->SetStatus(404);
        response->SetBody("404 Not Found");
        try {
            if (server->GetErrors()[404] != NULL) server->GetErrors()[404](req, response);
        } catch (std::exception e) {
            perror("Link: No 404 error handler found!");
        }
    }

    std::string res = response->GetVersion() + " " + std::to_string(response->GetStatus()) + " " + Link::Status(response->GetStatus()) + "\r\n";
    res = response->GetHeadersRaw() + "\r\n" + response->GetBody();

    if (server->IsDebugging()) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
        std::string color = "\033[0m";
        if (response->GetStatus() >= 200 && response->GetStatus() < 300) color = "\033[32m";
        else if (response->GetStatus() >= 300 && response->GetStatus() < 400) color = "\033[33m";
        else if (response->GetStatus() >= 400 && response->GetStatus() < 500) color = "\033[31m";
        else if (response->GetStatus() >= 500 && response->GetStatus() < 600) color = "\033[35m";
        std::cout << "\033[36m[Link]" << color << " [" << req->GetMethod() << "] " << req->GetPath() << " \033[35m" << std::setprecision(2) << time << "s" << "\033[0m" << std::endl;
    }

    int error = 0;
    socklen_t len = sizeof(error);
    int retval = getsockopt(this->sock, SOL_SOCKET, SO_ERROR, &error, &len);

    if (error != 0) {
        this->server->Status = 5;
    } else {
        int ret = Write(res.c_str(), res.length());
    }

    if (this->sslEnabled) {
        SSL_clear(ssl);
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(sock);

    if (server->IsMultiThreaded()) pthread_exit(NULL);
}

Link::Thread::Thread() {}
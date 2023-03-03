#include <Link.hpp>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>

int Link::Thread::Write(const void* buf, size_t count) {
    if (this->sslEnabled) return SSL_write((SSL*)ssl, buf, count);
    else return write(sock, buf, count);
}

int Link::Thread::Read(void* buf, size_t count) {
    if (this->sslEnabled) return SSL_read((SSL*)ssl, buf, count);
    else return read(sock, buf, count);
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

void Link::Thread::Run() {
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

    std::string body;
    if (req->GetHeader("Content-Length") != "") {
        for (int x=0;x<std::stoi(req->GetHeader("Content-Length"));x++) {
            Read(buffer, 1);
            body += buffer[0];
        }
    }

    req->SetBody(body);

    Link::Response* response = new Link::Response();
    response->SetHeader("Content-Type", "text/html");
    if (req->GetMethod() == "" || req->GetPath() == "") {
        response->SetStatus(400);
        response->SetBody("400 Bad Request");
        std::string res = response->GetVersion() + " " + std::to_string(response->GetStatus()) + " " + Link::Status(response->GetStatus()) + "\r\n";
        res = response->GetHeadersRaw() + "\r\n" + response->GetBody();
        Write(res.c_str(), res.length());
        if (server->IsMultiThreaded()) pthread_exit(NULL);
    }

    std::vector<std::string> staticPages = server->GetStaticPages();

    bool found = false;

    for (auto route : server->GetCallbacks()) {
        if (route.first[0] == req->GetMethod()) {
            std::string routePath = route.first[1];
            std::string reqPath = req->GetPath();
            if (routePath == reqPath) {
                found = true;
                try {
                    route.second(req, response);
                } catch (std::exception e) {
                    perror("Link: Error while executing route!");
                }
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
    Write(res.c_str(), res.length());
    if (server->IsMultiThreaded()) pthread_exit(NULL);
}
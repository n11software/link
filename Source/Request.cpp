/* 
 *  ______________________________________
 * |  _   _  _  _                         |
 * | | \ | |/ |/ |     Date: 06/24/2022   |
 * | |  \| |- |- |     Author: Levi Hicks |
 * | |     || || |                        |
 * | | |\  || || |                        |
 * | |_| \_||_||_|     File: Request.cpp  |
 * |                                      |
 * |                                      |
 * | Please do not remove this header.    |
 * |______________________________________|
 */

#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <filesystem>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <stdexcept>
#include <iomanip>
#include <cstring>
#include <chrono>
#include <vector>
#include <algorithm>
#include "../Includes/Link/Request.hpp"
#include "../Includes/Link/HTTPTools.hpp"

Request::Request(int sock, sockaddr_in* addr, std::string protocol, std::string path, std::string method, std::string request, std::map<std::string, std::string> queries) {
  this->sock = sock;
  this->addr = addr;
  this->path = path;
  this->method = method;
  this->request = request;
  this->queries = queries;
  this->protocol = protocol;
  {
    std::string header;
    std::istringstream stream(decodeHTTP(request));
    while(getline(stream, header)) {
      if (header.find(":") != std::string::npos && header.find_first_of(":")+2 < header.length()) {
        std::string key = header.substr(0, header.find_first_of(":"));
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
        std::string value = header.substr(header.find_first_of(":")+2);
        this->headers[key] = sanitize(value);
      } else if (header.substr(0, 3) == "GET" || header.substr(0, 4) == "POST" || header.substr(0, 3) == "PUT" || header.substr(0, 6) == "DELETE") {}
      else this->body += header + "\n";
    }
  }
  if (this->headers["connection"] == "keep-alive") {
    int time = 7200, interval = 60, retry = 3, opt = 1;
    if (setsockopt(this->sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0 || setsockopt(this->sock, IPPROTO_TCP, TCP_KEEPIDLE, &time, sizeof(time)) < 0 || 
        setsockopt(this->sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0 || setsockopt(this->sock, IPPROTO_TCP, TCP_KEEPCNT, &retry, sizeof(retry)) < 0) {
      close(this->sock);
    }
  }
  if (this->body.size()>0) this->body = this->body.substr(2, this->body.length()-3);
  if (this->method == "POST" && headers["content-type"].find("application/x-www-form-urlencoded") != std::string::npos) {
    std::string parameter;
    std::vector<std::string> arr;
    size_t pos = 0;
    std::string token;
    while ((pos = body.find("&")) != std::string::npos) {
      token = body.substr(0, pos);
      if (token != "") arr.push_back(token);
      body.erase(0, pos + 1);
    }
    if (body != "") arr.push_back(body);
    for (std::string parameter : arr) {
      std::string key = parameter.substr(0, parameter.find_first_of("="));
      std::string value = parameter.substr(parameter.find_first_of("=")+1);
      this->params[key] = sanitize(value);
    }
  }
}

std::string Request::GetProtocolVersion() {
  return this->protocol;
}

std::string Request::GetPath() {
  return this->path;
}

int Request::GetSocket() {
  return this->sock;
}

std::string Request::GetMethod() {
  return this->method;
}

std::string Request::GetRequest() {
  return this->request;
}

std::string Request::GetHeader(std::string header) {
  return this->headers[header];
}

std::string Request::GetQuery(std::string query) {
  return this->queries[query];
}

std::string Request::GetBody() {
  return this->body;
}

std::string Request::GetFormParam(std::string param) {
  return this->params[param];
}

struct sockaddr_in* Request::GetAddress() {
  return this->addr;
}
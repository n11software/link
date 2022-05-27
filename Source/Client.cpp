#include "../Includes/Client.hpp"
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
#include "../Includes/Log.hpp"

std::map<std::string, std::string> Headers;

void* ClientHandler(void* arg) {
  Client* Data = (Client*)arg;
  std::string Type = "", Path;
  {
    std::string Header;
    std::istringstream DataStream(Data->getRequest());
    while(getline(DataStream, Header)) {
      if (Header.substr(0, 3) == "GET" || Header.substr(0, 4) == "POST") {
        Type = Header.substr(0, 3) == "GET" ? "GET" : Header.substr(0, 4) == "POST" ? "POST" : "";
        Path = Header.substr(Type.length()+1);
        Path = Path.substr(0, Path.find_last_of("HTTP/1.1")-8);
      } else if (Header.find(":") != std::string::npos && Header.find_first_of(":")+2 < Header.length()) {
        std::string Key = Header.substr(0, Header.find_first_of(":"));
        std::transform(Key.begin(), Key.end(), Key.begin(), [](unsigned char c) { return std::tolower(c); });
        std::string Value = Header.substr(Header.find_first_of(":")+2);
        Headers[Key] = Value;
      }
    }
  }
  if (Headers["connection"] == "keep-alive") {
    int time = 7200, interval = 60, retry = 3, opt = 1;
    if (setsockopt(Data->getClientSocket(), SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0 || setsockopt(Data->getClientSocket(), IPPROTO_TCP, TCP_KEEPIDLE, &time, sizeof(time)) < 0 || 
        setsockopt(Data->getClientSocket(), IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0 || setsockopt(Data->getClientSocket(), IPPROTO_TCP, TCP_KEEPCNT, &retry, sizeof(retry)) < 0) {
      return NULL;
    }
  }
  std::string ResponseHeaders = "";
  std::string ResponseData = "Error: 404 Not Found";
  bool IsDirectory = false;
  std::string PathExtension = Path.substr(Path.find_last_of(".") + 1);
  if (PathExtension == "html" || PathExtension == "css" || PathExtension == "js" || PathExtension == "txt") ResponseHeaders = "Content-Type: text/" + (PathExtension != "js" ? PathExtension : "javascript") + "\n";
  else if (PathExtension == "png") ResponseHeaders = "Content-Type: image/png\n";
  else if (PathExtension == "jpg" || PathExtension == "jpeg") ResponseHeaders = "Content-Type: image/jpeg\n";
  else if (PathExtension == "gif") ResponseHeaders = "Content-Type: image/gif\n";
  else if (PathExtension == "ico") ResponseHeaders = "Content-Type: image/x-icon\n";
  else if (PathExtension == "svg") ResponseHeaders = "Content-Type: image/svg+xml\n";
  else if (PathExtension == "json") ResponseHeaders = "Content-Type: application/json\n";
  else if (Path.find(".") == std::string::npos) {
    ResponseHeaders = "Content-Type: text/html\n";
    IsDirectory = true;
  }
  else ResponseHeaders = "Content-Type: text/plain\n";
  if (Path == "/") Path = "/index";
  std::ifstream File(IsDirectory ? "www" + Path + ".html": "www/" + Path);
  if (File.good() && File.is_open()) {
    std::stringstream Buffer;
    Buffer << File.rdbuf();
    ResponseData = Buffer.str();
  }
  File.close();
  std::string Response = "HTTP/1.1 200 OK\n" + ResponseHeaders + "\n" + ResponseData;
  write(Data->getClientSocket(), Response.c_str(), Response.size());
  close(Data->getClientSocket());
  pthread_exit(NULL);
  return NULL;
}

Client::Client(int ClientSocket, struct sockaddr_in* ClientAddress) {
  this->ClientSocket = ClientSocket;
  this->ClientAddress = ClientAddress;
  recv(ClientSocket, Buffer, 1024, 0);
  this->Request = std::string(Buffer);
}

int Client::getClientSocket() { return ClientSocket; }
struct sockaddr_in* Client::getClientAddress() { return ClientAddress; }
std::string Client::getRequest() { return Request; }
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
#include <chrono>
#include <vector>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "../Includes/SHA512.hpp"
#include "../Includes/Log.hpp"

std::string HTTPDecode(std::string &SRC) {
  std::replace(SRC.begin(), SRC.end(), '+', ' ');
  std::string ret;
  char ch;
  int i, ii;
  for (i=0; i<SRC.length(); i++) {
    if (int(SRC[i])=='%') {
      sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
      ch=static_cast<char>(ii);
      ret+=ch;
      i=i+2;
    } else {
      ret+=SRC[i];
    }
  }
  return (ret);
}


sql::Driver* driver;
sql::Connection* con;

void InitializeDatabase() {
  try {
    driver = get_driver_instance();
    con = driver->connect("tcp://localhost:3306", "N11", "password"); // Please enter your MySQL password here
    con->setSchema("N11");
  } catch (sql::SQLException &e) {
    // GitHub Copilot
    std::cout << e.what() << std::endl;
    std::cout << "Error: SQLException in " << __FILE__;
    std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
    std::cout << "Error: " << e.what();
    std::cout << " (MySQL error code: " << e.getErrorCode();
    std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    exit(-1);
  }
}

sql::ResultSet* Query(std::string sql) {
  sql::ResultSet *res;
  try {
    sql::Statement* Statement = con->createStatement();
    res = Statement->executeQuery(sql);
    delete Statement;
    return res;
  } catch (sql::SQLException e) {
    InitializeDatabase();
    return Query(sql);
  }
}

void Execute(std::string sql) {
  try {
    sql::Statement* Statement = con->createStatement();
    Statement->executeUpdate(sql);
    delete Statement;
  } catch (sql::SQLException e) {
    InitializeDatabase();
    Execute(sql);
  }
}

std::map<std::string, std::string> Headers;

void* ClientHandler(void* arg) {
  std::chrono::_V2::system_clock::time_point StartTime = std::chrono::system_clock::now();
  Client* Data = (Client*)arg;
  std::string Type = "", Path, Body;
  {
    std::string Header, Request = Data->getRequest();
    std::istringstream DataStream(HTTPDecode(Request));
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
      } else {
        Body += Header + "\n";
      }
    }
  }
  if (Body.size()>0) Body = Body.substr(2, Body.length()-3);
  std::map<std::string, std::string> Params;
  if (Type == "POST") {
    std::string ContentType = Headers["content-type"];
    if (ContentType.find("application/x-www-form-urlencoded") != std::string::npos) {
      std::string Data = Body;
      std::istringstream DataStream(Data);
      std::string Param;
      while(getline(DataStream, Param, '&')) {
        std::string Key = Param.substr(0, Param.find_first_of("="));
        std::string Value = Param.substr(Param.find_first_of("=")+1);
        std::string newValue = "";
        for (int i=0; i<Value.length(); i++) {
          if (Value[i] == '\'') {
            newValue += "\\'";
          } else {
            newValue += Value[i];
          }
        }
        Params[Key] = newValue;
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
  std::string ResponseData = "Error: 404 Not Found", ResponseCode = "404 Not Found";
  bool IsDirectory = false;
  if (Path.substr(0, 4) != "/api") {
    std::string PathExtension = Path.substr(Path.find_last_of(".") + 1);
    if (PathExtension == "html" || PathExtension == "css" || PathExtension == "js" || PathExtension == "txt") ResponseHeaders = "Content-Type: text/" + (PathExtension != "js" ? PathExtension : "javascript") + "\n";
    else if (PathExtension == "png") ResponseHeaders = "Content-Type: image/png\n";
    else if (PathExtension == "jpg" || PathExtension == "jpeg") ResponseHeaders = "Content-Type: image/jpeg\n";
    else if (PathExtension == "gif") ResponseHeaders = "Content-Type: image/gif\n";
    else if (PathExtension == "ico") ResponseHeaders = "Content-Type: image/x-icon\n";
    else if (PathExtension == "svg") ResponseHeaders = "Content-Type: image/svg+xml\n";
    else if (PathExtension == "json") ResponseHeaders = "Content-Type: application/json\n";
    else if (PathExtension == "otf") ResponseHeaders = "Content-Type: font/otf\n";
    else if (Path.find(".") == std::string::npos) {
      ResponseHeaders = "Content-Type: text/html\n";
      IsDirectory = true;
    }
    else ResponseHeaders = "Content-Type: text/plain\n";
    if (Path == "/") Path = "/index";
    std::ifstream File(IsDirectory ? "www" + Path + ".html": "www/" + Path);
    if (File.good() && File.is_open()) {
      ResponseCode = "200 OK";
      std::stringstream Buffer;
      Buffer << File.rdbuf();
      ResponseData = Buffer.str();
    } else {
      ResponseCode = "404 Not Found";
    }
    File.close();
  } else {
    ResponseHeaders = "Content-Type: application/json\n";
    ResponseCode = "404 Not Found";
    if (Path == "/api/login") {
      if (Type == "POST") {
        std::string UUID = "";
        sql::ResultSet *res = Query("SELECT * FROM users WHERE email='" + Params["email"] + "' AND password='" + SHA512::hash(Params["password"]) + "'");
        std::cout << "SELECT * FROM users WHERE email='" + Params["email"] + "' AND password='" + SHA512::hash(Params["password"]) + "'" << std::endl;
        while (res->next()) UUID = res->getString("uuid");
        delete res;
        if (UUID == "") {
          ResponseCode = "401 Unauthorized";
          ResponseData = "{\"error\":\"Wrong email or password\"}";
        } else {
          srand(time(0)*getpid()*Data->getClientSocket());
          std::string Token = "";
          for (int i = 0; i < 32; i++) Token += "0123456789abcdef"[rand() % 16];
          ResponseData = "{\"Token\": \"" + Token +"\"}";
          std::string TimeDate;
          time_t rawtime;
          struct tm * timeinfo;
          time(&rawtime);
          timeinfo = localtime(&rawtime);
          char buffer[80];
          strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S", timeinfo);
          TimeDate = buffer;
          ResponseCode = "200 OK";
          Execute("INSERT INTO tokens (uuid, token, ip, os, browser, date) VALUES ('" + UUID + "', '" + Token + "', '" + std::string(inet_ntoa(Data->getClientAddress()->sin_addr)) + "', '" + Params["os"] + "', '" + Params["browser"] + "', '" + TimeDate + "')");
        }
      } else {
        ResponseCode = "400 Bad Request";
        ResponseData = "{\"error\":\"Invalid request method\"}";
      }
    } else if (Path == "/api/user") {
      if (Type == "POST") {
        if (Headers["authorization"].length() != 40) {
          ResponseCode = "401 Unauthorized";
          ResponseData = "{\"error\":\"Invalid token\"}";
          std::cout << Headers["authorization"] << std::endl;
        } else {
          std::string uuid = "", token;
          for (int i=0; i<Headers["authorization"].length(); i++) {
            if (Headers["authorization"][i] == '\'') {
              token += "\\'";
            } else {
              token += Headers["authorization"][i];
            }
          }
          token = token.substr(7, token.length()-8);
          sql::ResultSet *res = Query("SELECT * FROM tokens WHERE token='" + token + "'");
          while (res->next()) uuid = res->getString("uuid");
          res = Query("SELECT * FROM users WHERE uuid='" + uuid + "'");
          bool exists = false;
          while (res->next()) {
            exists = true;
            ResponseData = "{\"uuid\":\"" + res->getString("uuid") + "\",\"email\":\"" + res->getString("email") + "\"}";
            ResponseCode = "200 OK";
          }
          delete res;
          if (!exists) {
            ResponseCode = "401 Unauthorized";
            ResponseData = "{\"error\":\"Invalid token\"}";
          }
        }
      } else {
        ResponseCode = "400 Bad Request";
        ResponseData = "{\"error\":\"Invalid request method\"}";
      }
    } else if (Path.substr(0, 20) == "/api/user/pfp?email=") {
      if (Type == "GET") {
        std::string uuid = "", email = Path.substr(20);
        sql::ResultSet *res = Query("SELECT * FROM users WHERE email='" + email + "'");
        while (res->next()) uuid = res->getString("uuid");
        delete res;
        if (uuid == "") {
          ResponseCode = "404 Not Found";
        } else {
          std::string Path = "pfp/" + uuid + ".png";
          std::ifstream File(Path);
          if (File.good() && File.is_open()) {
            ResponseCode = "200 OK";
            std::stringstream Buffer;
            Buffer << File.rdbuf();
            ResponseData = Buffer.str();
            ResponseHeaders = "Content-Type: image/png\n";
          } else {
            std::string Path = "pfp/default.jpg";
            std::ifstream File(Path);
            if (File.good() && File.is_open()) {
              ResponseCode = "200 OK";
              std::stringstream Buffer;
              Buffer << File.rdbuf();
              ResponseData = Buffer.str();
              ResponseHeaders = "Content-Type: image/jpeg\n";
            } else {
              ResponseCode = "404 Not Found";
            }
          }
          File.close();
        }
      } else {
        ResponseCode = "400 Bad Request";
        ResponseData = "{\"error\":\"Invalid request method\"}";
      }
    }
  }
  ResponseHeaders += "Connection: " + Headers["connection"] + "\n";
  std::string Response = "HTTP/1.1 " + ResponseCode + "\n" + ResponseHeaders + "\n" + ResponseData;
  int packet = 0;
  while (packet < Response.size()) {
    int packetLength = Response.size()<packet+1024? Response.size()-packet: 1024;
    std::string PacketData = Response.substr(packet, packetLength);
    int sent = send(Data->getClientSocket(), PacketData.c_str(), PacketData.size(), 0);
    Log::Debug("Sent " + std::to_string(sent) + " bytes");
    if (sent < 0) {
      Log::Error("Error sending data to client");
      return NULL;
    }
    packet+=packetLength;
  }
  close(Data->getClientSocket());
  std::chrono::_V2::system_clock::time_point EndTime = std::chrono::system_clock::now();
  std::chrono::microseconds microseconds = std::chrono::duration_cast<std::chrono::microseconds>(EndTime-StartTime);
  Log::Info("\033[36mRequest from \033[32m" + std::string(inet_ntoa(Data->getClientAddress()->sin_addr)) + " \033[34m" + Type + " \033[33m" + Path + "\033[36m took \033[32m" + std::to_string(microseconds.count()) + "ms\033[0m");
  Log::Debug("Sent " + std::to_string(packet) + " bytes total");
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
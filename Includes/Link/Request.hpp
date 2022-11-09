/* 
 *  ______________________________________
 * |  _   _  _  _                         |
 * | | \ | |/ |/ |     Date: 06/24/2022   |
 * | |  \| |- |- |     Author: Levi Hicks |
 * | |     || || |                        |
 * | | |\  || || |                        |
 * | |_| \_||_||_|     File: Request.hpp  |
 * |                                      |
 * |                                      |
 * | Please do not remove this header.    |
 * |______________________________________|
 */

#pragma once
#include <iostream>
#include <map>
#include <netinet/in.h>

class Request {
  public:
    Request(int sock, sockaddr_in* addr, std::string protocol, std::string path, std::string method, std::string request, std::map<std::string, std::string> queries);
    std::string GetRequest();
    std::string GetPath();
    std::string GetProtocolVersion();
    std::string GetMethod();
    std::string GetHeader(std::string header);
    std::string GetQuery(std::string query);
    std::string GetBody();
    std::string GetFormParam(std::string param);
    int GetSocket();
    struct sockaddr_in* GetAddress();
  private:
    int sock;
    struct sockaddr_in* addr;
    std::map<std::string, std::string> headers, queries, params;
    std::string protocol, path, method, request, body;
};
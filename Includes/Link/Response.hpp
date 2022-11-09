/* 
 *  ______________________________________
 * |  _   _  _  _                         |
 * | | \ | |/ |/ |     Date: 06/25/2022   |
 * | |  \| |- |- |     Author: Levi Hicks |
 * | |     || || |                        |
 * | | |\  || || |                        |
 * | |_| \_||_||_|     File: Response.hpp |
 * |                                      |
 * |                                      |
 * | Please do not remove this header.    |
 * |______________________________________|
 */

#pragma once
#include <iostream>
#include "Request.hpp"

class Response {
  public:
    Response(Request* request, std::map<int, std::function<void(Request*, Response*)>>* errorHandlers);
    bool IsCancelled();
    void SetCancelled(bool cancelled);
    std::string GetHTTP();
    void SetHTTP(std::string http);
    void Send(std::string body);
    void SendFile(std::string path);
    void SetHeader(std::string header, std::string value);
    std::string GetHeader(std::string header);
    void RemoveHeader(std::string header);
    void SetStatus(std::string status);
    std::string GetStatus();
    void Error(int code);
  private:
    bool cancelled;
    std::string http, body, status = "200 OK";
    Request* request;
    std::map<std::string, std::string> headers;
    std::map<int, std::function<void(Request*, Response*)>>* errorHandlers;
};
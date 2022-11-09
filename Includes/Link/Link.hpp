/* 
 *  ______________________________________
 * |  _   _  _  _                         |
 * | | \ | |/ |/ |     Date: 06/23/2022   |
 * | |  \| |- |- |     Author: Levi Hicks |
 * | |     || || |                        |
 * | | |\  || || |                        |
 * | |_| \_||_||_|     File: Link.hpp     |
 * |                                      |
 * |                                      |
 * | Please do not remove this header.    |
 * |______________________________________|
 */

#pragma once
#include <functional>
#include <vector>
#include "Response.hpp"

class Link {
  public:
    Link(int port);
    void Default(std::function<void(Request*, Response*)> callback);
    void Get(std::string path, std::function<void(Request*, Response*)> callback);
    void Post(std::string path, std::function<void(Request*, Response*)> callback);
    void Put(std::string path, std::function<void(Request*, Response*)> callback);
    void Delete(std::string path, std::function<void(Request*, Response*)> callback);
    void Error(int code, std::function<void(Request*, Response*)> callback);
    int Start();
    int Stop();
    int GetPort();
  private:
    int port, sock;
    std::function<void(Request*, Response*)> defaultHandler;
    std::map<std::string, std::function<void(Request*, Response*)>> handlers;
    std::map<int, std::function<void(Request*, Response*)>> errorHandlers;
};
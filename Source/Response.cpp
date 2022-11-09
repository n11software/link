/* 
 *  ______________________________________
 * |  _   _  _  _                         |
 * | | \ | |/ |/ |     Date: 06/25/2022   |
 * | |  \| |- |- |     Author: Levi Hicks |
 * | |     || || |                        |
 * | | |\  || || |                        |
 * | |_| \_||_||_|     File: Response.cpp |
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
#include <functional>
#include "../Includes/Link/Response.hpp"

/*
 * Create a new Response object.
 */
Response::Response(Request* request, std::map<int, std::function<void(Request*, Response*)>>* errorHandlers) {
  this->cancelled = false;
  this->http = "";
  this->request = request;
  this->errorHandlers = errorHandlers;
}

/*
 * Check if the Response has been cancelled.
 */
bool Response::IsCancelled() {
  return this->cancelled;
}

/*
 * Set the Response to be cancelled.
 */
void Response::SetCancelled(bool cancelled) {
  this->cancelled = cancelled;
}

/*
 * Gets the HTTP response.
 */
std::string Response::GetHTTP() {
  return this->http;
}

/*
 * Sets the HTTP response.
 */
void Response::SetHTTP(std::string http) {
  this->http = http;
}

/*
 * Sends the HTTP response with the given body.
 */
void Response::Send(std::string body) {
  http = "HTTP/"+this->request->GetProtocolVersion()+" " + status + "\n";
  if (headers.size() > 0) for (std::_Rb_tree_iterator<std::pair<const std::string, std::string>> it = headers.begin(); it != headers.end(); it++) http += it->first + ": " + it->second + "\n";
  http += "\n" + body;
}

/*
 * Sends a file to the client.
 *
 * @param file The file path to send.
 */
void Response::SendFile(std::string path) {
  std::ifstream file(path);
  if (!file.is_open()||!file.good()) {
    this->Error(404);
    return;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string body = buffer.str();
  http = "HTTP/"+this->request->GetProtocolVersion()+" " + status + "\n";
  if (headers.size() > 0) for (std::_Rb_tree_iterator<std::pair<const std::string, std::string>> it = headers.begin(); it != headers.end(); it++) http += it->first + ": " + it->second + "\n";
  http += "\n"+body;
}

/*
 * Set a specific custom header.
 *
 * @param header The header to set
 * @param value The value to set the header to
 */
void Response::SetHeader(std::string header, std::string value) {
  this->headers[header] = value;
}

/*
 * Get a specific custom header.
 *
 * @param header The header to get
 * @return The value of the header
 */
std::string Response::GetHeader(std::string header) {
  return this->headers[header];
}

/*
 * Remove a specific custom header.
 *
 * @param header The header to remove
 */
void Response::RemoveHeader(std::string header) {
  this->headers.erase(header);
}

/*
* Set the status of the response.
*
* @param status The status to set
*/
void Response::SetStatus(std::string status) {
  this->status = status;
}

/*
 * Get the status of the response.
 *
 * @return The status of the response
 */
std::string Response::GetStatus() {
  return this->status;
}

/*
 * Sends an error response to the client.
 *
 * @param code The error code to send
 */
void Response::Error(int code) {
  if (this->errorHandlers->find(code) != this->errorHandlers->end()) {
    this->errorHandlers->at(code)(this->request, this);
  }
}
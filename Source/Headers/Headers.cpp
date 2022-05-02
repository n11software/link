#include "Headers.hpp"
#include <iostream>
#include <string.h>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <fstream>

/*
 * Initialize Headers.
 */
Headers::Headers(int ClientSocket) {
  this->ClientSocket = ClientSocket;
}

/*
 * Send a message.
 */
void Headers::Send(std::string Message) {
  Body = Message;
  std::string Response = "HTTP/1.1 " + Status + "\n" + ResponseHeaders + "\n" + "Content-Length: " + std::to_string(Body.length()) + "\n\n" + Body;
  if (ResponseHeaders.length() == 0) Response = "HTTP/1.1 " + Status + "\n" + "Content-Length: " + std::to_string(Body.length()) + "\n\n" + Body;
  write(this->ClientSocket, Response.c_str(), Response.size());
  close(this->ClientSocket);
}

/*
 * Send a file.
 */
void Headers::SendFile(std::string FilePath) {
  std::ifstream File(FilePath);
  if (File.is_open()) {
    std::string extension = FilePath.substr(FilePath.find_last_of(".") + 1);
    if (extension == "html") {
      this->SetStatus("404 Not Found");
      this->Send("<h1>404 Not Found</h1>");
    } else if (extension == "css") ResponseHeaders = "Content-Type: text/css";
    else if (extension == "js") ResponseHeaders = "Content-Type: text/javascript";
    else if (extension == "png") ResponseHeaders = "Content-Type: image/png";
    else if (extension == "jpg") ResponseHeaders = "Content-Type: image/jpeg";
    else if (extension == "jpeg") ResponseHeaders = "Content-Type: image/jpeg";
    else if (extension == "gif") ResponseHeaders = "Content-Type: image/gif";
    else if (extension == "svg") ResponseHeaders = "Content-Type: image/svg+xml";
    else if (extension == "ico") ResponseHeaders = "Content-Type: image/x-icon";
    else if (extension == "json") ResponseHeaders = "Content-Type: application/json";
    else if (extension == "xml") ResponseHeaders = "Content-Type: application/xml";
    else if (extension == "pdf") ResponseHeaders = "Content-Type: application/pdf";
    else if (extension == "zip") ResponseHeaders = "Content-Type: application/zip";
    else if (extension == "txt") ResponseHeaders = "Content-Type: text/plain";
    else if (extension == "mp3") ResponseHeaders = "Content-Type: audio/mpeg";
    else if (extension == "mp4") ResponseHeaders = "Content-Type: video/mp4";
    else if (extension == "ogg") ResponseHeaders = "Content-Type: audio/ogg";
    else if (extension == "wav") ResponseHeaders = "Content-Type: audio/wav";
    else if (extension == "webm") ResponseHeaders = "Content-Type: video/webm";
    else if (extension == "woff") ResponseHeaders = "Content-Type: font/woff";
    else if (extension == "woff2") ResponseHeaders = "Content-Type: font/woff2";
    else if (extension == "ttf") ResponseHeaders = "Content-Type: font/ttf";
    else if (extension == "otf") ResponseHeaders = "Content-Type: font/otf";
    else ResponseHeaders = "Content-Type: text/plain";
    std::string FileContents((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
    this->Send(FileContents);
  } else {
    std::ifstream Backup(FilePath+".html");
    if (Backup.is_open()) {
      std::string FileContents((std::istreambuf_iterator<char>(Backup)), std::istreambuf_iterator<char>());
      this->Send(FileContents);
    } else {
      this->SetStatus("404 Not Found");
      this->Send("<h1>404 Not Found</h1>");
    }
  }
}

/*
 * Set the status.
 */
void Headers::SetStatus(std::string Status) {
  this->Status = Status;
}

/*
 * Set the request headers.
 */
void Headers::SetRequest(std::string Request) {
  this->RequestHeaders = Request;
}

/*
 * Set the response headers.
 */
void Headers::SetResponse(std::string Response) {
  this->ResponseHeaders = Response;
}
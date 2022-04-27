#ifndef Headers
#include <iostream>
#include <string.h>
#include <vector>

/*
 * HTTP Headers.
 */
class Headers {
  public:
    Headers(int ClientSocket);
    void Send(std::string Message),
    SendFile(std::string FilePath), 
    SetStatus(std::string Status),
    SetRequest(std::string Request),
    SetResponse(std::string Response);
  private:
    int ClientSocket;
    std::string Status = "200 OK", RequestHeaders = "", ResponseHeaders = "", Body = "";
};

#endif
#ifndef Client
#include <iostream>

class Client {
  public:
    Client(int ClientSocket, struct sockaddr_in* ClientAddress);
    int getClientSocket();
    struct sockaddr_in* getClientAddress();
    std::string getRequest();
  private:
    int ClientSocket;
    struct sockaddr_in* ClientAddress;
    std::string Request;
    char Buffer[1024] = { 0 };
};

void InitializeDatabase();
void* ClientHandler(void* arg);

#endif
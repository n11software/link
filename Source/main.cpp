#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <filesystem>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../Includes/Log.hpp"
#include "../Includes/Client.hpp"

int main() {
  Log::SetDebugMode(false);
  Log::SetVerboseMode(true);
  try {
    std::filesystem::create_directories("/var/log/link/");
    Log::SetSaveLogs(true);
    time_t now = time(0);
    tm *ltm = localtime(&now);
    Log::SetLogFile("/var/log/link/link_" + std::to_string(ltm->tm_year + 1900) + "-" + std::to_string(ltm->tm_mon + 1) + "-" + std::to_string(ltm->tm_mday) + ".log");
  } catch (std::filesystem::filesystem_error& e) {
    Log::Error("\033[31mLogs will not be stored please run as root!");
  }
  InitializeDatabase();
  int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (ServerSocket == 0) {
    Log::Error("\033[31mCould not create socket");
    return 1;
  }
  int opt = 1;
  if (setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    Log::Error("\033[31mThis port is still in use!");
    return 1;
  }
  int time = 7200, interval = 60, retry = 3;
  struct sockaddr_in ServerAddress;
  ServerAddress.sin_family = AF_INET;
  ServerAddress.sin_addr.s_addr = INADDR_ANY;
  ServerAddress.sin_port = htons(3000);
  if (bind(ServerSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0) {
    Log::Error("\033[31mCould not bind to port 80!");
    return 1;
  }
  if (listen(ServerSocket, 50) < 0) {
    Log::Error("\033[31mCould not listen to port 80!");
    return 1;
  }
  Log::Info("\033[36mListening on port \033[32m80");
  while (true) {
    char ClientData[1024] = {0};
    struct sockaddr_in ClientAddress;
    socklen_t ClientAddressSize = sizeof(ClientAddress);
    int ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddress, &ClientAddressSize);
    if (ClientSocket == -1) {
      Log::Error("\033[31mCould not accept connection from " + std::string(inet_ntoa(ClientAddress.sin_addr)));
    } else {
      Client client = Client(ClientSocket, &ClientAddress);
      pthread_t ClientThread;
      pthread_create(&ClientThread, NULL, ClientHandler, &client);
      pthread_join(ClientThread, NULL);
    }
  }
  shutdown(ServerSocket, SHUT_RDWR);
  return 0;
}
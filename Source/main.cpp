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
  Log::SetDebugMode(true);
  Log::SetVerboseMode(true);
  try {
    std::filesystem::create_directories("/var/log/link/");
    // GitHub Copilot Start
    Log::SetSaveLogs(true);
    time_t now = time(0);
    tm *ltm = localtime(&now);
    Log::SetLogFile("/var/log/link/link_" + std::to_string(ltm->tm_year + 1900) + "-" + std::to_string(ltm->tm_mon + 1) + "-" + std::to_string(ltm->tm_mday) + ".log");
    // GitHub Copilot End
  } catch (std::filesystem::filesystem_error& e) {
    fprintf(stderr, "Logs will not be stored please run as root!\n");
  }
  int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (ServerSocket == 0) {
    Log::Error("Could not create socket");
    return 1;
  }
  int opt = 1;
  if (setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    Log::Error("This port is still in use!");
    return 1;
  }
  int time = 7200, interval = 60, retry = 3;
  struct sockaddr_in ServerAddress;
  ServerAddress.sin_family = AF_INET;
  ServerAddress.sin_addr.s_addr = INADDR_ANY;
  ServerAddress.sin_port = htons(80);
  if (bind(ServerSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0) {
    Log::Error("Could not bind to port 80!");
    return 1;
  }
  if (listen(ServerSocket, 10) < 0) {
    Log::Error("Could not listen to port 80!");
    return 1;
  }
  Log::Info("Listening on port 80");
  while (true) {
    char ClientData[1024] = {0};
    struct sockaddr_in ClientAddress;
    socklen_t ClientAddressSize = sizeof(ClientAddress);
    int ClientSocket = accept(ServerSocket, (struct sockaddr*)&ClientAddress, &ClientAddressSize);
    if (ClientSocket < 0) {
      Log::Error("Could not accept connection from " + std::to_string(ClientAddress.sin_addr.s_addr));
    }
    Log::Info("Accepted connection from " + std::string(inet_ntoa(ClientAddress.sin_addr)));
    Client client = Client(ClientSocket, &ClientAddress);
    pthread_t ClientThread;
    pthread_create(&ClientThread, NULL, ClientHandler, &client);
    pthread_join(ClientThread, NULL);
  }
  shutdown(ServerSocket, SHUT_RDWR);
  return 0;
}
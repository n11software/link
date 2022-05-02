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
#include "ServerThread.hpp"
#include "../Headers/Headers.hpp"

/*
 * Manages HTTP connections. 
 */
void* ConnectionHandler(void* arg) {
  HTTP::ClientData* Data = (HTTP::ClientData*)arg;
  char msg[1000];
  Headers* req = new Headers(Data->getSocket());
  Headers* res = new Headers(Data->getSocket());
  recv(Data->getSocket(), msg, 1000, 0);
  req->SetRequest(msg);
  res->SetStatus("200 OK");
  std::string msgString(msg);
  std::string path = msgString.substr(0, msgString.find("\n")-10);
  if (path.substr(0,1) == "G") path = path.substr(5, path.length());
  if (path == "") path = "index";
  res->SendFile("www/" + path);
  close(Data->getSocket());
  pthread_exit(NULL);
  return NULL;
}

/*
 * Manages HTTP connections.
 *
 * Initialized via HTTP::ServerThread
 */
void run(void* arg) {
  HTTP::ServerThread* ServerThread = (HTTP::ServerThread*)arg;
  int ServerSocket = socket(PF_INET, SOCK_STREAM, 0), ClientSocket;
  struct sockaddr_in ServerAddress;
  struct sockaddr_storage ServerStorage;
  socklen_t AddressSize;
  ServerAddress.sin_family = AF_INET;
  ServerAddress.sin_port = htons(ServerThread->getPort());
  ServerAddress.sin_addr.s_addr = INADDR_ANY;
  bzero(&(ServerAddress.sin_zero), 8);
  if (bind(ServerSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) != 0) {
    fprintf(stderr, "[HTTP][!] Could not bind to port %d\n", ServerThread->getPort());
    exit(0);
  }
  listen(ServerSocket, ServerThread->getMaxConnections());
  fprintf(stdout, "[HTTP][#] Listening on port %d\n", ServerThread->getPort());
  pthread_t ThreadID[ServerThread->getMaxConnections()+ServerThread->getMaxQueuedConnections()];
  int CurrentThread = 0;
  while (ServerThread->isRunning()) {
    AddressSize = sizeof(ServerStorage);
    ClientSocket = accept(ServerSocket, (struct sockaddr*)&ServerStorage, &AddressSize);
    HTTP::ClientData ClientData(ServerThread, ClientSocket);
    pthread_create(&ThreadID[CurrentThread++], NULL, ConnectionHandler, &ClientData);
    if (CurrentThread >= ServerThread->getMaxQueuedConnections()) {
      CurrentThread = 0;
      while (CurrentThread < ServerThread->getMaxQueuedConnections()) pthread_join(ThreadID[CurrentThread++], NULL);
    }
  }
  shutdown(ServerSocket, SHUT_RDWR);
  pthread_exit(NULL);
}

namespace HTTP {
  /*
   * Manages the server connections.
   *
   * @param port The port to listen on.
   * @param maxConnections The maximum number of connections to accept.
   */
  ServerThread::ServerThread(int port, int maxConnections, int maxQueuedConnections, bool* Running) {
    this->port = port;
    this->maxConnections = maxConnections;
    this->maxQueuedConnections = maxQueuedConnections;
    this->Running = Running;
    std::thread Thread = std::thread(run, (void*)this);
    Thread.join();
  }

  /*
   * Gets the port.
   *
   * @return The port number.
   */
  int ServerThread::getPort() {
    return this->port;
  }

  /*
   * Gets the maximum number of connections.
   *
   * @return The maximum number of connections.
   */
  int ServerThread::getMaxConnections() {
    return this->maxConnections;
  }

  /*
   * Gets the maximum number of queued connections.
   *
   * @return The maximum number of queued connections.
   */
  int ServerThread::getMaxQueuedConnections() {
    return this->maxQueuedConnections;
  }

  /*
   * Gets if the main thread is still running.
   *
   * @return True if the main thread is still running.
   */
  bool ServerThread::isRunning() {
    return this->Running;
  }

  /*
   * Holds all the client's needed data.
   *
   * @param serverThread The server thread.
   * @param socket The socket.
   * @param address The address.
   */
  ClientData::ClientData(ServerThread* serverThread, int socket) {
    this->serverThread = serverThread;
    this->socket = socket;
  }

  /*
   * Gets the server thread.
   *
   * @return The server thread.
   * 
   */
  ServerThread* ClientData::getServerThread() {
    return this->serverThread;
  }

  /*
   * Gets the socket.
   *
   * @return The socket.
   */
  int ClientData::getSocket() {
    return this->socket;
  }
}
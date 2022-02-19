#include <iostream>
#include <thread>
#include "ServerThread.hpp"

/*
 * Manages HTTP connections.
 *
 * Initialized via HTTP::ServerThread
 */
void run(void* arg) {
  HTTP::ServerThread* ServerThread = (HTTP::ServerThread*)arg;
  std::cout << "[HTTP Server Thread] Running on port " << ServerThread->getPort() << " with " << ServerThread->getMaxConnections() << " max connections." << std::endl;
}

namespace HTTP {
  /*
   * Manages the server connections.
   *
   * @param port The port to listen on.
   * @param maxConnections The maximum number of connections to accept.
   */
  ServerThread::ServerThread(int port, int maxConnections) {
    this->port = port;
    this->maxConnections = maxConnections;
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
}
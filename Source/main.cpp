#include <iostream>
#include "HTTP/ServerThread.hpp"

bool Running = true;

void exiting() {
  Running = false;
}

int main() {
  std::atexit(exiting);
  std::cout << "[Initializing HTTP Server Thread]" << std::endl;
  HTTP::ServerThread serverThread(3000, 100, 10, &Running);
  return 0;
}
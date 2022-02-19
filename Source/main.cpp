#include <iostream>
#include "HTTP/ServerThread.hpp"

int main() {
  std::cout << "[Initializing HTTP Server Thread]" << std::endl;
  HTTP::ServerThread serverThread(80, 100);
  return 0;
}
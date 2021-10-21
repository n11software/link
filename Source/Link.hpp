#pragma once
#include <iostream>
#include <string>
#include <regex>

void getArguments(int argc, char *argv[], std::string args[3]) {
  for (uint16_t i=0;i<=argc;i++) {
    if (argc > i && strcmp(argv[i], "-d") == 0) {
      args[0] = argv[i+1];
    } else if (argc > i && strcmp(argv[i], "-p") == 0) {
      args[1] = argv[i+1];
      if (!std::regex_match(args[1], std::regex("^((6553[0-5])|(655[0-2][0-9])|(65[0-4][0-9]{2})|(6[0-4][0-9]{3})|([1-5][0-9]{4})|([0-5]{0,5})|([0-9]{1,4}))$"))) {
        std::cout << "Please specify a valid port!" << std::endl;
      }
    } else if (argc > i && strcmp(argv[i], "-t") == 0) {
      args[2] = argv[i+1];
      if (!strspn(args[2].c_str(), "0123456789") == args[2].size()) {
        std::cout << "Please specify a valid amount of threads!" << std::endl;
      }
    }
  }
}

class LinkServer {
  public:
    LinkServer();
};
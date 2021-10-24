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
#include <pthread.h>
#include <fstream>
#include "Link.hpp"
#include "Config.hpp"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char msg[1000];

std::string locationParser(std::string location) {
  if (location.find(".") == std::string::npos) {
    if (location.substr(location.length()-1, location.length()) == "/") return "/index.html";
    else return location + ".html";
  }
  return location;
}

void* thread(void* arg) {
  int newSocket = *((int*) arg);
  recv(newSocket, msg, 1000, 0);
  std::string location, type, status;
  std::string req = msg; {
    std::string line;
    std::istringstream f(req);  
    int i = 0;
    while (getline(f, line)) {
      if (i == 0) {
        if (line.substr(0,1) == "G") {
          location = line.substr(4, line.length()-14);
          location = locationParser(location);
          if (location.substr(location.find(".")+1) == "html") {
            type = "text/html";
          } else if (location.substr(location.find(".")+1) == "css") {
            type = "text/css";
          } else if (location.substr(location.find(".")+1) == "csv") {
            type = "text/csv";
          } else if (location.substr(location.find(".")+1) == "ico") {
            type = "image/x-icon";
          } else {
            type = "text/plain";
          }
        } else if (line.substr(0,1) == "H") {
          std::cout << "Head ";
        } else if (line.substr(0,2) == "PO") {
          std::cout << "Post ";
        } else if (line.substr(0,2) == "PU") {
          std::cout << "Put ";
        } else if (line.substr(0,1) == "D") {
          std::cout << "Delete ";
        } else if (line.substr(0,1) == "C") {
          std::cout << "Connect ";
        } else if (line.substr(0,1) == "O") {
          std::cout << "Options ";
        } else if (line.substr(0,1) == "T") {
          std::cout << "Trace ";
        } else if (line.substr(0,2) == "PA") {
          std::cout << "Patch ";
        } else {
          close(newSocket);
          pthread_exit(NULL);
        }
      }
      i++;
    }
  }

  std::cout << req << std::endl;

  std::string data;
  pthread_mutex_lock(&lock); {
    std::string line;
    std::ifstream file(config.getDirectory() + location, std::fstream::binary);
    if (file.is_open()) {
      while (std::getline(file, line)) {
        data += line + '\n';
      }
      status = "200";
    } else {
      std::ifstream file404(config.getDirectory() + "404.html", std::fstream::binary);
      if (file404.is_open()) {
        line="";
        while (std::getline(file404, line)) {
          data += line + '\n';
        }
      } else data = "404 File Not Found!";
      status = "404";
    }
    file.close();
  }

  pthread_mutex_unlock(&lock);
  std::string length = std::to_string(data.length());
  std::string res = "HTTP/1.1 " + status + " OK\nAccept-Ranges: bytes\nContent-Type: " + type + "\nContent-Length: " + length + "\n\n" + data;
  send(newSocket, res.data(), res.length(), 0);
  close(newSocket);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  config = Config("config");
  int serverSocket, newSocket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(config.getPort());
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  if (listen(serverSocket, config.getHTTPThreads()) == 0) printf("Listening\n");
  else
    printf("Error\n");
    pthread_t tid[config.getHTTPThreads()+config.getLobbyThreads()];
    int i = 0;
    while(1) {
      addr_size = sizeof serverStorage;
      newSocket = accept(serverSocket, (struct sockaddr*) &serverStorage, &addr_size);
      if (pthread_create(&tid[i++], NULL, thread, &newSocket) != 0) printf("Failed to create thread\n");
      if( i >= config.getHTTPThreads()) {
        i = 0;
        while(i < config.getHTTPThreads()) pthread_join(tid[i++],NULL);
        i = 0;
      }
    }
  return 0;
}
#pragma once
#include <iostream>
#include <vector>

void Split(std::string s, std::vector<std::string> &v){
  std::string temp = "";
  for(int i=0;i<s.length();++i){
    if(s[i]=='='){
      v.push_back(temp);
      temp = "";
    }
    else{
      temp.push_back(s[i]);
    }
  }
  v.push_back(temp);
}

class Config {
  public:
    Config() {}
    Config(std::string path) {
      std::string line;
      std::ifstream file(path, std::fstream::binary);
      if (file.is_open()) {
        while (std::getline(file, line)) {
          this->data += line + '\n';
          line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
          std::vector<std::string> v;
          Split(line, v);
          for (int i = 0;i<v.size();i+=1) {
            if (v[i] == "DocumentRoot") directory = v[i+1];
            if (v[i] == "Port") port = std::stoi(v[i+1]);
            if (v[i] == "Threads") threads = std::stoi(v[i+1]);
          }
        }
        file.close();
      } else {
        std::cout << "The config file does not exist please create it!" << std::endl;
      }
    }
    std::string getData() { return this->data; }
    std::string getDirectory() { return this->directory; }
    int getPort() { return port; }
    int getThreads() { return this->threads; }
  private:
    std::string data, directory;
    int port, threads;
};

Config config;
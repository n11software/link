#include <Link.hpp>
#include <iostream>
#include <fstream>

std::string replace(std::string data, std::string delimiter, std::string replacement) {
  size_t pos = 0;
  while ((pos = data.find(delimiter)) != std::string::npos) {
    data.replace(pos, delimiter.length(), replacement);
  }
  return data;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }
    Link::Request request(argv[1]);
    Link::Client client(&request);
    Link::Response* response = client.Send();

    std::cout << response->GetBody() << std::endl;

    return 0;
}
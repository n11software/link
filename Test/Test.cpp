#include <Link.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }
    Link::Request request(argv[1]);
    std::cout << request.GetURL() << std::endl;
    std::cout << request.GetProtocol() << std::endl;
    std::cout << request.GetDomain() << std::endl;
    std::cout << request.GetPath() << std::endl;

    Link::Client client(&request);
    client.Send();

    return 0;
}
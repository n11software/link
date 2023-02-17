#include <Link.hpp>
#include <iostream>

int main() {
    Link::Request request("https://www.google.com/");
    std::cout << request.GetURL() << std::endl;
    std::cout << request.GetProtocol() << std::endl;
    std::cout << request.GetDomain() << std::endl;
    std::cout << request.GetPath() << std::endl;

    Link::Client client(&request);
    client.Send();

    return 0;
}
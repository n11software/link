#include <Link.hpp>
#include <iostream>

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
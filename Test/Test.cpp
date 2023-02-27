#include <Link.hpp>
#include <iostream>
#include <fstream>

int main() {
    {
        Link::Request request("https://google.com/");
        Link::Client client(&request);
        Link::Response* response = (Link::Response*)malloc(sizeof(Link::Response));
        response = client.Send();

        std::cout << response->GetBody() << std::endl;

        free(response);
    }

    {
        Link::Request request("https://example.com/");
        Link::Client client(&request);
        Link::Response* response = (Link::Response*)malloc(sizeof(Link::Response));
        response = client.Send();

        std::cout << response->GetBody() << std::endl;

        free(response);
    }
    return 0;
}
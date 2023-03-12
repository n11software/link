#include <Link.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    bool server = false;
    bool https = false;
    if (argc > 1) {
        if (std::string(argv[1]) == "server") server = true;
        if (std::string(argv[1]) == "https") https = true;
    }
    if (server) {
        Link::Server server(8080);
        server.SetStaticPages("public/");
        server.Get("/", [](Link::Request* request, Link::Response* response) {
            if (request->GetParam("message") != "") response->SetBody(request->GetParam("message"));
            else response->SetBody("Hello, world!");
        });
        server.Get("/:message", [](Link::Request* request, Link::Response* response) {
            response->SetBody(request->GetParam("message"));
        });
        server.EnableSSL("certificate.pem", "key.pem");
        server.SetStartMessage("Server started on port 8080");
        server.Start();
    } else {
        Link::Request* request = new Link::Request(https?"https://localhost/":"http://localhost/");
        request->SetPath("/login");
        Link::Client client(request);
        client.SetPort(8080);
        Link::Response* response = client.Send();
        std::cout << response->GetBody() << std::endl;
    }

    return 0;
}
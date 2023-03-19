#include <Link.hpp>
#include <iostream>
#include <fstream>
#include <openssl/ssl.h>
#include <thread>

bool ready = false;
bool ssl = false;

void* client(void* arg) {
    while (!ready) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Link::Request* req = new Link::Request(std::string(ssl?"https":"http") + "://localhost:8080/");
    Link::Client client(req);
    Link::Response* res = client.Send();
    if (res->GetStatus() == 200 && res->GetBody() == "Hello World!") {
        std::cout << "\033[1;32mTest passed" << std::endl;
    } else {
        std::cout << "\033[1;31mTest failed" << std::endl;
    }
    ready = false;
    return nullptr;
}

Link::Server server(8080);
int main(int argc, char** argv) {
    if (SSLeay() < 0x30000000L) {
        std::cout << "OpenSSL version is lower than 3.0.0" << std::endl;
        return 0;
    }

    std::thread t;
    t = std::thread(client, nullptr);
    t.detach();
    

    server.Get("/", [](Link::Request* req, Link::Response* res) {
        res->SetBody("Hello World!");
        server.Stop();
    });

    ready = true;
    server.Start();

    server.EnableMultiThreading();

    t = std::thread(client, nullptr);
    t.detach();

    ready = true;

    server.Start();

    ssl = true;

    server.EnableSSL("certificate.pem", "key.pem");

    t = std::thread(client, nullptr);
    t.detach();

    ready = true;

    server.Start();

    server.DisableMultiThreading();

    t = std::thread(client, nullptr);
    t.detach();

    ready = true;

    server.Start();

    return 0;
}

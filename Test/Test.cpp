#include <Link.hpp>
#include <iostream>
#include <fstream>
#include <openssl/ssl.h>
#include <thread>

bool ready = false;
bool ssl = false;

Link::Server server(8080);

void* client(void* arg) {
    while (!ready || !server.IsRunning()) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Link::Request* req = new Link::Request(std::string(ssl?"https":"http") + "://localhost:8080/");
    Link::Client client(req);
    Link::Response* res = client.Send();
    if (client.Status==0&&res->GetStatus() == 200) {
        std::cout << "\033[1;32mTest passed " << res->GetBody() << std::endl;
    } else {
        std::cout << "\033[1;31mTest failed" << std::endl;
    }
    return nullptr;
}

int main(int argc, char** argv) {
    if (SSLeay() < 0x30000000L) {
        std::cout << "OpenSSL version is lower than 3.0.0" << std::endl;
        return 0;
    }

    // 1/4
    std::thread t;
    t = std::thread(client, nullptr);
    t.detach();

    server.Get("/", [](Link::Request* req, Link::Response* res) {
        res->SetBody("(SSL: " + std::string(ssl?"true":"false") + " | Multi-Threaded: " + std::string(server.IsMultiThreaded()?"true":"false") + ")");
        server.Stop();
    });

    ready = true;
    server.Start();
    if (server.Status != 0) {
        std::cout << "\033[1;31mTest failed" << std::endl;
        return 0;
    }

    // 2/4
    server.EnableMultiThreading();
    t = std::thread(client, nullptr);
    t.detach();
    server.Start();
    if (server.Status != 0) {
        std::cout << "\033[1;31mTest failed" << std::endl;
        return 0;
    }

    // 3/4
    ssl = true;
    server.EnableSSL("certificate.pem", "key.pem");
    t = std::thread(client, nullptr);
    t.detach();
    server.Start();
    if (server.Status != 0) {
        std::cout << "\033[1;31mTest failed" << std::endl;
        return 0;
    }

    // 4/4
    server.DisableMultiThreading();
    t = std::thread(client, nullptr);
    t.detach();
    server.Start();
    if (server.Status != 0) {
        std::cout << "\033[1;31mTest failed" << std::endl;
        return 0;
    }

    return 0;
}

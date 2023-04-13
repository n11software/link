#include <Link.hpp>
#include <iostream>
#include <vector>
#include <thread>

std::vector<Link::Proxy*> proxies;

void ProxyMiddleware(Link::Request* request, Link::Response* response, Link::Server* server) {
    std::string host = request->GetRawHeader("Host").substr(request->GetRawHeader("Host").find(":")+2, request->GetRawHeader("Host").length()-2);
    if (host == "") {
        response->SetBody("No Host header!")->Close()->SetStatus(400);
        return;
    }
    
    std::string target = "";

    for (auto proxy: proxies) { // Sorry for killing your computer
        for (auto t: proxy->GetTargets()) {
            for (auto h: t->GetHosts()) {
                if (h == host) {
                    target = t->GetTarget();
                    break;
                }
            }
        }
    }

    if (target == "") {
        response->SetBody("This domain is not proxied!")->Close()->SetStatus(404);
        return;
    }
    std::string path = request->GetPath();
    if (request->GetParams().size() > 0) {
        path += "?";
        for (auto param : request->GetParams()) {
            path += param.first + "=" + param.second + "&";
        }
        path = path.substr(0, path.length()-1);
    }
    Link::Request* req = new Link::Request(target + path);
    req->SetHeadersRaw(request->GetRawHeaders());
    req->SetRawHeader("Host", target);
    req->SetPath(path);
    req->SetBody(request->GetBody());
    Link::Client client(req);
    Link::Response* res = client.Send();
    if (client.Status == 0) {
        response->SetHeadersRaw(res->GetHeadersRaw());
        response->SetBody(res->GetBody())->Close();
    } else {
        response->SetBody("Proxy failed!")->Close()->SetStatus(500);
        std::cout << "Error: " << client.Status << std::endl;
    }
}

Link::Proxy::Proxy() {
    this->targets = std::vector<Target*>();
}

Link::Proxy::Proxy(std::vector<Target*> targets) {
    this->targets = targets;
}

Link::Proxy* Link::Proxy::AddTarget(Target* target) {
    this->targets.push_back(target);
    return this;
}

void* http(void* arg) {
    Link::Server server(80);
    server.Use(ProxyMiddleware);
    server.EnableMultiThreading();
    server.Start();
    return nullptr;
}

Link::Proxy* Link::Proxy::Start() {
    Link::Server server(443);
    server.Use(ProxyMiddleware);
    server.EnableMultiThreading();
    std::thread t;
    t = std::thread(http, nullptr);
    t.detach();
    server.Start();
    return this;
}

Link::Proxy* Link::Proxy::EnableHTTPRedirects() {
    this->redirects = true;
    return this;
}

bool Link::Proxy::Redirects() {
    return this->redirects;
}

std::vector<Link::Target*> Link::Proxy::GetTargets() {
    return this->targets;
}
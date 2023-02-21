#include <Link.hpp>
#include <iostream>
#include <sstream>

Link::Response::Response(std::string header, std::string body) {
    this->SetHeadersRaw(header)->SetBody(body);
}

Link::Response* Link::Response::SetHeader(std::string key, std::string value) {
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    this->headers[key] = value;
    return this;
}

Link::Response* Link::Response::SetBody(std::string body) {
    this->body = body;
    return this;
}

Link::Response* Link::Response::SetHeadersRaw(std::string headersRaw) {
    this->headersRaw = headersRaw;
    std::string line = headersRaw.substr(0, headersRaw.find("\r\n"));
    this->version = line.substr(0, line.find(" "));
    line = line.substr(line.find(" ") + 1);
    this->status = std::stoi(line.substr(0, line.find(" ")));
    
    // get headers line by line by using getline
    std::istringstream iss(headersRaw.substr(headersRaw.find("\r\n") + 2));
    std::string l;
    while (std::getline(iss, l)) {
        if (l == "") break;
        std::string key = l.substr(0, l.find(":"));
        std::string value = l.substr(l.find(":") + 2);
        this->SetHeader(key, value);
    }
    
    return this;
}

Link::Response* Link::Response::SetStatus(int status) {
    this->status = status;
    return this;
}

Link::Response* Link::Response::SetVersion(std::string version) {
    this->version = version;
    return this;
}

std::string Link::Response::GetHeader(std::string key) {
    std::string lower = key;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return this->headers[lower];
}

std::string Link::Response::GetBody() {
    return this->body;
}

std::string Link::Response::GetHeadersRaw() {
    return this->headersRaw;
}

std::string Link::Response::GetVersion() {
    return this->version;
}

int Link::Response::GetStatus() {
    return this->status;
}
#include <Link.hpp>

Link::Request::Request() {
    this->SetURL("")->SetMethod("GET");
}

Link::Request::Request(std::string url) {
    this->SetURL(url)->SetMethod("GET");
}

Link::Request* Link::Request::SetURL(std::string url) {
    this->url = url;
    this->protocol = url.substr(0, url.find("://"));
    this->domain = url.substr(url.find("://") + 3);
    this->domain = this->domain.substr(0, this->domain.find("/"));
    this->path = url.substr(url.find("://") + 3);
    this->path = this->path.substr(this->path.find("/"));
    this->headers["Host"] = url;
    return this;
}

Link::Request* Link::Request::SetMethod(std::string method) {
    this->headers["Method"] = method;
    return this;
}

Link::Request* Link::Request::SetHeader(std::string key, std::string value) {
    this->headers[key] = value;
    return this;
}

Link::Request* Link::Request::SetCookie(std::string key, std::string value) {
    this->cookies[key] = value;
    return this;
}

Link::Request* Link::Request::SetParam(std::string key, std::string value) {
    this->params[key] = value;
    return this;
}

Link::Request* Link::Request::SetBody(std::string body) {
    this->body = body;
    return this;
}

Link::Request* Link::Request::SetPath(std::string path) {
    if (path[0] != '/') path = "/" + path;
    this->SetURL(this->protocol + "://" + this->domain + path);
    return this;
}

Link::Request* Link::Request::SetProtocol(std::string protocol) {
    this->SetURL(protocol + "://" + this->domain + this->path);
    return this;
}

Link::Request* Link::Request::SetDomain(std::string domain) {
    this->SetURL(this->protocol + "://" + domain + this->path);
    return this;
}

std::string Link::Request::GetPath() {
    return this->path;
}

std::string Link::Request::GetProtocol() {
    return this->protocol;
}

std::string Link::Request::GetDomain() {
    return this->domain;
}

std::string Link::Request::GetURL() {
    return this->headers["Host"];
}

std::string Link::Request::GetMethod() {
    return this->headers["Method"];
}

std::string Link::Request::GetHeader(std::string key) {
    return this->headers[key];
}

std::string Link::Request::GetCookie(std::string key) {
    return this->cookies[key];
}

std::string Link::Request::GetParam(std::string key) {
    return this->params[key];
}

std::string Link::Request::GetBody() {
    return this->body;
}

std::string Link::Request::GetRawHeaders() {
    std::string res = "";
    for (auto it = this->headers.begin(); it != this->headers.end(); it++) res += it->first + ": " + it->second + "\r\n";
    return res;
}

std::string Link::Request::GetRawParams() {
    std::string res = "";
    for (auto it = this->params.begin(); it != this->params.end(); it++) res += it->first + "=" + it->second + "&";
    return res.substr(0, res.length() - 1);
}

std::string Link::Request::GetRawBody() {
    return this->body;
}
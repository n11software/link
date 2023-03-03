#include <Link.hpp>
#include <iostream>
#include <sstream>

Link::Request::Request(std::string headers, std::string body) {
    this->SetHeadersRaw(headers)->SetBody(body);
}

std::string decodeHTTP(std::string &src) {
    std::replace(src.begin(), src.end(), '+', ' ');
    std::string ret;
    char ch;
    int i, ii;
    for (i=0;i<src.length();i++) {
        if (int(src[i])=='%') {
            switch (src[i+1]) {
                case '0'...'9':
                case 'a'...'f':
                case 'A'...'F':
                    break;
                default:
                    ret += '%';
                    continue;
            }
            sscanf(src.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else ret+=src[i];
    }
    return (ret);
}

Link::Request* Link::Request::SetHeadersRaw(std::string headersRaw) {
    std::string line;
    std::stringstream stream(headersRaw);
    int i = 0;
    while (std::getline(stream, line)) {
        if (i == 0) {
            this->SetMethod(line.substr(0, line.find(" ")));
            this->SetVersion(line.substr(line.find("HTTP/") + 5));
            std::string path = line.substr(line.find(" ") + 1);
            this->SetPath(path.substr(0, path.find(" ")));
        } else {
            this->SetHeader(line.substr(0, line.find(": ")), line.substr(line.find(": ") + 2));
        }
        i++;
    }
    if (this->GetHeader("Host") != "") this->SetURL(this->GetHeader("Host") + this->path);
    return this;
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
    this->method = method;
    return this;
}

Link::Request* Link::Request::SetHeader(std::string key, std::string value) {
    std::string lower = key;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    this->headers[lower] = value;
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
    path = decodeHTTP(path);
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
    return this->method;
}

std::string Link::Request::GetHeader(std::string key) {
    std::string lower = key;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return this->headers[lower];
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
    res += this->method + " " + this->path + " " + this->version + "\r\n";
    for (auto it = this->params.begin(); it != this->params.end(); it++) res += it->first + "=" + it->second + "&";
    return res.substr(0, res.length() - 1);
}

std::string Link::Request::GetRawBody() {
    return this->body;
}

std::string Link::Request::GetVersion() {
    return this->version;
}

Link::Request* Link::Request::SetVersion(std::string version) {
    this->version = version;
    return this;
}
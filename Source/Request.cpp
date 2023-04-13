#include <Link.hpp>
#include <iostream>
#include <sstream>

Link::Request::Request(std::string headers, std::string body) {
    std::string line;
    std::stringstream stream(headers);
    std::map<std::string, std::string> headerMap;
    int i = 0;
    std::string path;
    while (std::getline(stream, line)) {
        if (i == 0) {
            path = line.substr(line.find(" ") + 1);
            path = path.substr(0, path.find(" "));
        } else {
            std::string val = line.substr(line.find(": ") + 2);
            headerMap[line.substr(0, line.find(": "))] = val.substr(0, val.length()-1);
        }
        i++;
    }
    this->SetURL("http://"+headerMap["Host"]+path);
    this->SetHeadersRaw(headers)->SetBody(body);
    this->SetRawHeader("Host", headerMap["Host"]);
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

std::map<std::string, std::string> Link::Request::GetParams() {
    return this->params;
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
            this->SetRawHeader(line.substr(0, line.find(": ")), line.substr(line.find(": ") + 2, line.find("\r")));
        }
        i++;
    }
    if (this->path.find("?") != std::string::npos) {
        std::string queries = this->path.substr(this->path.find("?") + 1);
        std::stringstream stream(queries);
        while (std::getline(stream, line, '&')) {
            std::string key = line.substr(0, line.find("="));
            std::string value = line.substr(line.find("=") + 1);
            this->SetParam(decodeHTTP(key), decodeHTTP(value));
        }
        SetPath(this->path.substr(0, this->path.find("?")));
    }
    if (this->GetHeader("Cookie") != "") {
        std::string cookies = this->GetHeader("Cookie");
        std::stringstream stream(cookies);
        while (std::getline(stream, line, ';')) {
            std::string key = line.substr(0, line.find("="));
            if (key[0] == ' ') key = key.substr(1);
            std::string value = line.substr(line.find("=") + 1);
            this->SetCookie(decodeHTTP(key), decodeHTTP(value));
        }
    }
    return this;
}

Link::Request::Request(std::string url) {
    this->version = "1.1";
    this->port = 0;
    this->SetURL(url)->SetMethod("GET");
}

Link::Request* Link::Request::SetURL(std::string url) {
    this->url = url;
    this->protocol = url.substr(0, url.find("://"));
    this->domain = url.substr(url.find("://") + 3);
    this->domain = this->domain.substr(0, this->domain.find("/"));
    if (port == 0 && this->domain.find(":") == std::string::npos) {
        this->SetPort(this->protocol == "https"?443:80);
    } else if (port == 0) {
        this->SetPort(std::stoi(this->domain.substr(this->domain.find(":") + 1)));
        this->domain = this->domain.substr(0, this->domain.find(":"));
    }
    this->path = url.substr(url.find("://") + 3);
    this->path = this->path.substr(this->path.find("/"));
    this->headers["Host"] = domain + (port == 80?"":":" + std::to_string(port));
    return this;
}

Link::Request* Link::Request::SetPort(int d) {
    this->port = d;
    return this;
}

int Link::Request::GetPort() {
    return port;
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
    return this->url;
}

std::string Link::Request::GetMethod() {
    return this->method;
}

std::string Link::Request::GetHeader(std::string key) {
    std::string lower = key;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return this->headers[lower];
}

std::string Link::Request::GetRawHeader(std::string key) {
    return key + ": " + this->headers[key];
}

Link::Request* Link::Request::SetRawHeader(std::string key, std::string value) {
    this->headers[key] = value;
    return this;
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
    std::string headers = this->GetMethod() + " " + this->GetURL()
                .substr(this->GetURL().find(this->domain)+this->domain.length() + 
                (this->port == 80 || this->port == 443?0:std::to_string(this->port).length() + 1))
                + " HTTP/" + this->GetVersion() + "\r\n";
    for (auto const& x : this->headers) {
        if (x.second != "") headers += x.first + ": " + x.second + "\r\n";
    }
    return headers;
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

Link::Request* Link::Request::SetIP(std::string ip) {
    this->ip = ip;
    return this;
}

std::string Link::Request::GetIP() {
    return this->ip;
}
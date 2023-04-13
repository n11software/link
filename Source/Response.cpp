#include <Link.hpp>
#include <iostream>
#include <sstream>
#include <zlib.h>

Link::Response::Response() {
    this->SetHeadersRaw("HTTP/1.1 200 OK\r\n")->SetBody("");
    this->SetInstanceType("Server");
    this->closed = false;
}

Link::Response::Response(std::string header, std::string body) {
    this->SetHeadersRaw(header);
    this->SetBody(body);
    this->SetInstanceType("Client");
    this->closed = false;
}

Link::Response* Link::Response::SetInstanceType(std::string type) {
    this->instanceType = type;
    return this;
}

bool Link::Response::InstanceOf(std::string type) {
    return this->instanceType == type;
}

Link::Response* Link::Response::Close() {
    this->closed = true;
    return this;
}

bool Link::Response::isClosed() {
    return this->closed;
}

Link::Response* Link::Response::SetHeader(std::string key, std::string value) {
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    this->headers[key] = value;
    return this;
}

std::string decompress(std::string data) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)data.data();
    inflateInit2(&strm, 16 + MAX_WBITS);
    char outbuffer[32768];
    std::string outstring;
    do {
        strm.avail_out = 32768;
        strm.next_out = (Bytef*)outbuffer;
        inflate(&strm, Z_NO_FLUSH);
        if (outstring.size() < strm.total_out) {
            outstring.append(outbuffer, strm.total_out - outstring.size());
        }
    } while (strm.avail_out == 0);
    inflateEnd(&strm);
    return outstring;
}

std::string deflate(std::string data) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)data.data();
    deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    char outbuffer[32768];
    std::string outstring;
    do {
        strm.avail_out = 32768;
        strm.next_out = (Bytef*)outbuffer;
        deflate(&strm, Z_FINISH);
        if (outstring.size() < strm.total_out) {
            outstring.append(outbuffer, strm.total_out - outstring.size());
        }
    } while (strm.avail_out == 0);
    deflateEnd(&strm);
    return outstring;
}

Link::Response* Link::Response::SetBody(std::string body) {
    this->SetHeader("Content-Length", std::to_string(body.size()));
    this->SetHeader("Transfer-Encoding", "");
    this->body = body;
    if (this->InstanceOf("Server")) {
        this->SetHeader("Content-Encoding", "");
        return this;
    }
    if (this->GetHeader("Content-Encoding") == "gzip") {
        this->body = decompress(body);
        return this;
    }
    if (this->GetHeader("Content-Encoding") == "deflate") {
        this->body = deflate(body);
        return this;
    }
    return this;
}

Link::Response* Link::Response::SetRawHeader(std::string key, std::string value) {
    this->headers[key] = value;
    return this;
}

Link::Response* Link::Response::SetHeadersRaw(std::string headersRaw) {
    this->headersRaw = headersRaw;
    std::string line = headersRaw.substr(0, headersRaw.find("\r\n"));
    this->version = line.substr(0, line.find(" "));
    line = line.substr(line.find(" ") + 1);
    this->status = std::stoi(line);
    
    std::istringstream iss(headersRaw.substr(headersRaw.find("\r\n") + 2));
    std::string l;
    while (std::getline(iss, l)) {
        if (l == "") break;
        std::string key = l.substr(0, l.find(":"));
        std::string value = l.substr(l.find(":") + 2);
        value = value.substr(0, value.find("\r"));
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
    std::string headers = this->version + " " + std::to_string(this->status) + " " + Link::Status(this->status) + "\r\n";
    for (auto const& x : this->headers) {
        if (x.second != "") headers += x.first + ": " + x.second + "\r\n";
    }
    return headers;
}

std::string Link::Response::GetVersion() {
    return this->version;
}

int Link::Response::GetStatus() {
    return this->status;
}
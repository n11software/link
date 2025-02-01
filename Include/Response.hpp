#pragma once
#include <string>
#include <map>
#include <sstream>

namespace Link {

class Response {
public:
    Response();
    explicit Response(const std::string& raw_response);
    void send(const std::string& body);
    void status(int code);
    void setHeader(const std::string& key, const std::string& value);
    void json(const std::string& jsonString);
    void redirect(const std::string& url);
    
    // Add these helper methods
    std::string serialize() const;
    bool isSent() const { return sent; }
    const std::string& getBody() const { return body; }
    std::string getHeadersRaw() const {
        std::string raw_headers;
        for (const auto& [key, value] : headers) {
            raw_headers += key + ": " + value + "\r\n";
        }
        return raw_headers;
    }
    void sendFile(std::string filename);
    std::string retrieveCache(std::string filename);
    int getStatusCode() const { return statusCode; }

private:
    int statusCode;
    std::map<std::string, std::string> headers;
    std::string body;
    bool sent;
    void parseRawResponse(const std::string& raw);
    std::string decompress_gzip(const std::string& compressed);
    std::string decompress_deflate(const std::string& compressed);
    std::string decompress_bzip2(const std::string& compressed);
    std::string decompress_brotli(const std::string& compressed);
    std::map<std::string, std::string> cachedFiles;
};

} // namespace Link
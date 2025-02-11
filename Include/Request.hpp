#pragma once
#include <iostream>
#include <map>
#include "URL.hpp"
#include "CookieManager.hpp"

namespace Link {

struct MultipartFile {
    std::string filename;
    std::string content_type;
    std::string content;
};

class Request {
    public:
        Request(const std::string& request, std::string ip, std::string protocol);
        std::string getMethod() const;
        URL getURL() const;
        std::string getVersion() const;
        std::string getHeader(const std::string& key) const;
        std::string getBody() const;
        std::string getIP() const;
        std::string getCookie(const std::string& key) const { return cookieManager.getCookie(key); }
        bool hasCookie(const std::string& key) const { return cookieManager.hasCookie(key); }
        const CookieManager& getCookieManager() const { return cookieManager; }
        std::string getParam(const std::string& key) const;
        void setParam(const std::string& key, const std::string& value); // Internal use
        std::string getHeadersRaw() const {
            std::string raw_headers;
            for (const auto& [key, value] : headers) {
                raw_headers += key + ": " + value + "\r\n";
            }
            return raw_headers;
        }
        const std::map<std::string, std::string>& getHeaders() const { return headers; }
        const MultipartFile* getFile(const std::string& name) const {
            auto it = files.find(name);
            return it != files.end() ? &it->second : nullptr;
        }
    private:
        std::string method, protocol;
        std::string url;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
        std::string ip;
        std::map<std::string, std::string> params; // Add path parameters
        CookieManager cookieManager;
        std::map<std::string, MultipartFile> files;
        void parseMultipartFormData(const std::string& boundary);
        std::string extractBoundary(const std::string& content_type) const;
        friend class Server; // Allow Server to set params
};
} // namespace Link
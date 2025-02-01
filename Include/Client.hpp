#pragma once
#include <string>
#include <map>
#include <memory>
#include "Link.hpp"

namespace Link {

class Client {
public:
    Client(bool enableSSL = true);
    ~Client();

    // Configuration
    Client& setVerifyPeer(bool verify);
    Client& setTimeout(int seconds);
    
    // Request builders
    Response Get(const std::string& url);
    Response Post(const std::string& url, const std::string& body = "");
    Response Put(const std::string& url, const std::string& body = "");
    Response Delete(const std::string& url);
    
    // Header management
    Client& setHeader(const std::string& key, const std::string& value);
    Client& clearHeaders();
    std::string getHeadersRaw() const;
    std::string getLastRequestRaw() const;  // Add this method

private:
    class ClientImpl;
    std::unique_ptr<ClientImpl> impl;
};

} // namespace Link
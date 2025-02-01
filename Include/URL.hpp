#pragma once
#include <iostream>
#include <map>

class URL {
    public:
        URL() = default;
        URL(const std::string& url);
        operator std::string() const; // Add string cast operator
        std::string getProtocol() const;
        std::string getHost() const;
        std::string getPath() const;
        std::string getQuery() const;
        std::string getFragment() const;
        std::string getQueryParam(const std::string& key) const;
        std::string getScheme() const;
        std::string getPort() const;

    private:
        std::string protocol;
        std::string host;
        std::string path;
        std::string query;
        std::string fragment;
        std::map<std::string, std::string> queryParams;
        std::string scheme;
        std::string port;
        void parseQueryParams(const std::string& queryString);
        void parseUrl(const std::string& url);  // Add this method
};
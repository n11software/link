#pragma once
#include <string>
#include <vector>
#include <map>

namespace Link {
    
    class Request {

        public:
            Request();
            Request(std::string url);

            Link::Request* SetURL(std::string url),
                         * SetMethod(std::string method),
                         * SetHeader(std::string key, std::string value),
                         * SetCookie(std::string key, std::string value),
                         * SetParam(std::string key, std::string value),
                         * SetBody(std::string body),
                         * SetPath(std::string path),
                         * SetProtocol(std::string protocol),
                         * SetDomain(std::string domain);
                         
            std::string GetURL(),
                        GetMethod(),
                        GetHeader(std::string key),
                        GetCookie(std::string key),
                        GetParam(std::string key),
                        GetBody(),
                        GetPath(),
                        GetProtocol(),
                        GetDomain();
            
            std::string GetRawHeaders(), GetRawParams(), GetRawBody();
        private:
            std::map<std::string, std::string> headers, cookies, params;
            std::string body, protocol, path, domain, url;

    };

    class Response {

        public:
            
        private:

    };

    class Client {

        public:
            Client(Request* request);
            void Send();

            Request* SetPort(int port);
            int GetPort();

            Request* GetRequest();
            Response* GetResponse();
        private:
            Request* request;
            Response* response;
            int port = 0;

    };

}
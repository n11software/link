#pragma once
#include <string>
#include <vector>
#include <map>
#include <openssl/ssl.h>

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
            Response(std::string headers, std::string body);

            Response* SetHeader(std::string key, std::string value),
                    * SetBody(std::string body),
                    * SetHeadersRaw(std::string headersRaw),
                    * SetStatus(int status),
                    * SetVersion(std::string version);

            std::string GetHeader(std::string key),
                        GetBody(),
                        GetHeadersRaw(),
                        GetVersion();
            int GetStatus();
        private:
            std::map<std::string, std::string> headers;
            std::string body, headersRaw, version;
            int status;
    };

    class Client {

        public:
            Client(Request* request);
            Link::Response* Send();

            Request* SetPort(int port),
                   * SetRequest(Request* request);
            int GetPort();

            Request* GetRequest();
            Response* GetResponse();
        private:
            int Write(const void* buf, size_t count);
            int Read(void* buf, size_t count);
            bool getChunkSize(int& remaining, std::string& body);
            Request* request;
            Response* response;
            SSL* ssl;
            int port, sock;

    };

}
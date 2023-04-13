#pragma once
#include <string>
#include <vector>
#include <map>
#include <openssl/ssl.h>
#include <functional>

namespace Link {
    
    class Request {

        public:
            Request(std::string headers, std::string body);
            Request(std::string url);

            Link::Request* SetURL(std::string url),
                         * SetMethod(std::string method),
                         * SetHeader(std::string key, std::string value),
                         * SetCookie(std::string key, std::string value),
                         * SetParam(std::string key, std::string value),
                         * SetBody(std::string body),
                         * SetPath(std::string path),
                         * SetProtocol(std::string protocol),
                         * SetDomain(std::string domain),
                         * SetVersion(std::string version),
                         * SetHeadersRaw(std::string headersRaw),
                         * SetRawHeader(std::string key, std::string value),
                         * SetIP(std::string ip),
                         * SetPort(int port);
                         
            std::string GetURL(),
                        GetMethod(),
                        GetHeader(std::string key),
                        GetRawHeader(std::string key),
                        GetCookie(std::string key),
                        GetParam(std::string key),
                        GetBody(),
                        GetPath(),
                        GetProtocol(),
                        GetDomain(),
                        GetVersion(),
                        GetIP();

            std::map<std::string, std::string> GetParams();

            int GetPort();
            
            std::string GetRawHeaders(), GetRawParams(), GetRawBody();
        private:
            std::map<std::string, std::string> headers, cookies, params;
            std::string body, protocol, path, domain, url, method, version, ip;
            int port;

    };

    class Response {

        public:
            Response();
            Response(std::string headers, std::string body);

            Response* SetHeader(std::string key, std::string value),
                    * SetBody(std::string body),
                    * SetHeadersRaw(std::string headersRaw),
                    * SetRawHeader(std::string key, std::string value),
                    * SetStatus(int status),
                    * SetVersion(std::string version),
                    * Close(),
                    * SetInstanceType(std::string type);

            std::string GetHeader(std::string key),
                        GetBody(),
                        GetHeadersRaw(),
                        GetVersion();
            int GetStatus();
            bool isClosed();
            bool InstanceOf(std::string type);
        private:
            std::map<std::string, std::string> headers;
            std::string body, headersRaw, version, instanceType;
            bool closed;
            int status;
    };

    class Client {

        public:
            Client(Request* request);
            Link::Response* Send();

            Request* SetRequest(Request* request);

            Request* GetRequest();
            Response* GetResponse();
            int Status;
        private:
            int Write(const void* buf, size_t count);
            int Read(void* buf, size_t count);
            bool getChunkSize(int& remaining, std::string& body);
            Request* request;
            Response* response;
            SSL* ssl;
            int sock;

    };

    class Server {

        public:
            Server();
            Server(int port);

            Server* SetPort(int port),
                  * Start(),
                  * Stop(),
                  * EnableMultiThreading(),
                  * DisableMultiThreading(),
                  * EnableDebugging(),
                  * EnableSSL(std::string certPath, std::string keyPath),
                  * Get(std::string path, std::function<void(Request*, Response*)> callback),
                  * Post(std::string path, std::function<void(Request*, Response*)> callback),
                  * Route(std::string method, std::string path, std::function<void(Request*, Response*)> callback),
                  * Error(int status, std::function<void(Request*, Response*)> callback),
                  * SetStaticPages(std::string path),
                  * SetStartMessage(std::string message),
                  * Use(std::function<void(Request*, Response*, Server*)> middleware);

            int GetPort();
            std::map<std::vector<std::string>, std::function<void(Request*, Response*)>> GetCallbacks();
            std::vector<std::function<void(Request*, Response*, Server*)>> GetMiddlewares();
            std::vector<std::string> GetStaticPages();
            std::map<int, std::function<void(Request*, Response*)>> GetErrors();
            bool IsRunning(), IsMultiThreaded(), IsSSL(), IsDebugging();
            std::string GetStaticPagesDirectory();
            int Status;
        private:
            int port, sock;
            SSL_CTX* ctx;
            bool running, sslEnabled, multiThreaded, debugging;
            std::string certPath, keyPath, staticPages, startMessage;
            std::map<std::vector<std::string>, std::function<void(Request*, Response*)>> callbacks;
            std::vector<std::function<void(Request*, Response*, Server*)>> middlewares;
            std::map<int, std::function<void(Request*, Response*)>> errors;
    };

    class Thread {

        public:
            Thread();
            Thread(Server* server, int sock, bool sslEnabled);
            Thread(Server* server, SSL* ssl, bool sslEnabled);
            void SetIP(std::string ip), Run();
        private:
            int Write(const void* buf, size_t count);
            int Read(void* buf, size_t count);
            Server* server;
            SSL* ssl;
            bool sslEnabled;
            int sock;
            std::string ip;
    };

    class Target {
        public:
            Target(std::string host, std::string target);
            Target* AddHost(std::string host);
            std::vector<std::string> GetHosts();
            std::string GetTarget();
            bool Redirects();
        private:
            std::vector<std::string> hosts;
            std::string target;
    };

    class Proxy {
        public:
            Proxy();
            Proxy(std::vector<Target*> targets);
            Proxy* AddTarget(Target* target), *Start(), *EnableHTTPRedirects();
            std::vector<Target*> GetTargets();
            bool Redirects();
        private:
            std::vector<Target*> targets;
            bool redirects;
    };

    std::string Status(int status);
    std::string GetMIMEType(std::string path);

}

#pragma once
#include <functional>
#include <map>
#include <string>
#include "URL.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Client.hpp"

namespace Link {

class Response;
class Request;

using RouteCallback = std::function<void(const Request&, Response&)>;
using ErrorCallback = std::function<void(const Request&, Response&, int)>;

class Server {
public:
    Server(bool enableMetrics = false, 
          const std::string& certFile = "certificate.crt",
          const std::string& keyFile = "private.key");
    ~Server();

    // Add error handler methods
    void OnError(int statusCode, ErrorCallback callback);
    void OnError(ErrorCallback callback); // Default error handler

    // Route handlers
    void Get(const std::string& path, RouteCallback callback);
    void Post(const std::string& path, RouteCallback callback);
    void Put(const std::string& path, RouteCallback callback);
    void Delete(const std::string& path, RouteCallback callback);

    // Start server
    void Listen(int port);

private:
    struct ServerImpl;
    std::unique_ptr<ServerImpl> impl;
};

} // namespace Link
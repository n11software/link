#include <iostream>
#include <link>
#include <SHA512>

int main() {
  Link HTTP(80);
  HTTP.Error(404, [](Request* request, Response* response) {
    response->SetHTTP("HTTP/1.1 404 Not Found\r\n\r\n404 Not Found");
  });
  HTTP.Default([](Request* request, Response* response) {
    std::string path = request->GetPath();
    if (request->GetPath() == "/index") {
      response->Error(404);
      return;
    }
    if (request->GetPath() == "/") path = "/index";
    if (request->GetPath().substr(request->GetPath().find_last_of(".") + 1) == "html") {
      response->Error(404);
      return;
    }
    std::cout << "Request: " << path << std::endl;
    if (path.find(".") == std::string::npos) path += ".html";
    response->SendFile("../www" + path);
  });
  HTTP.Get("/api/user/pfp", [](Request* request, Response* response) {
    std::cout << "Tried to get pfp of " << request->GetQuery("email") << std::endl;
    response->Error(404);
  });
  std::cout << "Starting server..." << std::endl;
  HTTP.Start();
  return 0;
}
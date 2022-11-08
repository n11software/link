#include <iostream>
#include <link>

int main() {
  Link HTTP(3000);
  HTTP.Error(404, [](Request* request, Response* response) {
    response->SetHTTP("HTTP/2 404 Not Found\r\n\r\n404 Not Found");
  });
  HTTP.Default([](Request* request, Response* response) {
    response->Error(404);
  });
  HTTP.Get("/", [](Request* request, Response* response) {
    response->Send("<h1>Hello World!</h1>");
  });
  std::cout << "Starting server..." << std::endl;
  HTTP.Start();
  return 0;
}
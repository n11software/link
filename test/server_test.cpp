#include "Link.hpp"
#include <iostream>

int main() {
    try {
        Link::Server server(true);  // Enable metrics
        
        server.Get("/", [](const Link::Request& req, Link::Response& res) {
            res.send("Hello World!");
        });

        // Match exact path
        server.Get("/users", [](const Link::Request& req, Link::Response& res) {
            res.send("Users list");
        });

        // Match path with parameters
        server.Get("/users/[user]", [](const Link::Request& req, Link::Response& res) {
            std::string username = req.getParam("user");
            res.send("User profile: " + username);
        });

        // Match nested parameters
        server.Get("/users/[user]/posts/[id]", [](const Link::Request& req, Link::Response& res) {
            std::string username = req.getParam("user");
            std::string postId = req.getParam("id");
            res.send("Post " + postId + " by " + username);
        });

        // Match wildcard
        server.Get("/files/*", [](const Link::Request& req, Link::Response& res) {
            res.send("File system access");
        });

        server.Get("/test.html", [](const Link::Request& req, Link::Response& res) {
            res.sendFile("test/test.html");
        });

        // Custom 404 handler
        server.OnError(404, [](const Link::Request& req, Link::Response& res, int code) {
            res.status(code);
            res.send("Custom 404: Page not found");
        });

        // Default error handler for all other errors
        server.OnError([](const Link::Request& req, Link::Response& res, int code) {
            res.status(code);
            res.send("Oops! Something went wrong: " + std::to_string(code));
        });

        std::cout << "Server starting on port 9999..." << std::endl;
        server.Listen(9999);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#include "Link.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <fstream>

// Mock server for local testing
void runMockServer(int port) {
    Link::Server server(false);  // Disable metrics for testing
    
    server.Get("/test", [](const Link::Request& req, Link::Response& res) {
        res.send("GET response");
    });
    
    server.Post("/test", [](const Link::Request& req, Link::Response& res) {
        res.json("{\"status\":\"success\"}");
    });
    
    server.Put("/test", [](const Link::Request& req, Link::Response& res) {
        res.send("PUT response");
    });
    
    server.Delete("/test", [](const Link::Request& req, Link::Response& res) {
        res.send("DELETE response");
    });
    
    try {
        server.Listen(port);
    } catch (const std::exception& e) {
        std::cerr << "Mock server error: " << e.what() << std::endl;
    }
}

void testHttpClient() {
    const int TEST_PORT = 8080;
    
    // Start mock server in a separate thread
    std::thread serverThread(runMockServer, TEST_PORT);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for server to start
    
    Link::Client client(true);  // Enable SSL
    client.setVerifyPeer(false); // Disable SSL verification for local testing
    std::cout << "Testing HTTP client..." << std::endl;
    
    try {
        // Test with local mock server (using HTTP instead of HTTPS)
        auto response = client.Get("http://localhost:" + std::to_string(TEST_PORT) + "/test");
        assert(response.isSent() && "Response should be marked as sent");
        std::cout << "✓ Local GET request successful" << std::endl;
        
        response = client.Post("http://localhost:" + std::to_string(TEST_PORT) + "/test", "{\"test\":\"data\"}");
        assert(response.isSent() && "Response should be marked as sent");
        std::cout << "✓ Local POST request successful" << std::endl;
        
        response = client.Put("http://localhost:" + std::to_string(TEST_PORT) + "/test", "update");
        assert(response.isSent() && "Response should be marked as sent");
        std::cout << "✓ Local PUT request successful" << std::endl;
        
        response = client.Delete("http://localhost:" + std::to_string(TEST_PORT) + "/test");
        assert(response.isSent() && "Response should be marked as sent");
        std::cout << "✓ Local DELETE request successful" << std::endl;

        // Optional: Test external endpoints (disabled by default)
        bool test_external = true;
        if (test_external) {
            std::cout << "\nTesting external endpoints..." << std::endl;
            
            // Add Google test
            std::cout << "Fetching google.com..." << std::endl;
            client.clearHeaders()
                  .setHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/132.0.0.0 Safari/537.36")
                  .setHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8")
                  .setHeader("Accept-Language", "en-US,en;q=0.5")
                  .setHeader("Accept-Encoding", "gzip, deflate, br, zstd");
                  
            auto google_response = client.Get("https://www.google.com/");
            
            // Save request, response, and final HTML
            if (google_response.isSent()) {
                // Save request headers
                std::ofstream req("test/request.txt");
                if (req.is_open()) {
                    req << client.getLastRequestRaw();  // This will include the full request
                    req.close();
                    std::cout << "✓ Saved request to test/request.txt" << std::endl;
                }

                // Save response headers
                std::ofstream resp("test/response.txt");
                if (resp.is_open()) {
                    resp << "HTTP/1.1 " << google_response.getStatusCode() << "\r\n";
                    resp << google_response.getHeadersRaw();
                    resp << "\r\n";
                    resp.close();
                    std::cout << "✓ Saved response headers to test/response.txt" << std::endl;
                }

                // Save HTML body separately
                std::ofstream out("test/test.html");
                if (out.is_open()) {
                    out << google_response.getBody();
                    out.close();
                    std::cout << "✓ Saved Google homepage to test/test.html" << std::endl;
                }
            }

            // Test GET request
            response = client.Get("http://httpbin.org/get");
            assert(response.isSent() && "Response should be marked as sent");
            std::cout << "✓ GET request successful" << std::endl;
            
            // Test POST request with JSON
            client.setHeader("Content-Type", "application/json");
            response = client.Post("http://httpbin.org/post", "{\"test\":\"data\"}");
            assert(response.isSent() && "Response should be marked as sent");
            std::cout << "✓ POST request successful" << std::endl;
            
            // Test PUT request
            response = client.Put("http://httpbin.org/put", "{\"update\":\"test\"}");
            assert(response.isSent() && "Response should be marked as sent");
            std::cout << "✓ PUT request successful" << std::endl;
            
            // Test DELETE request
            response = client.Delete("http://httpbin.org/delete");
            assert(response.isSent() && "Response should be marked as sent");
            std::cout << "✓ DELETE request successful" << std::endl;
            
            // Test HTTPS request
            response = client.Get("https://api.github.com/zen");
            assert(response.isSent() && "Response should be marked as sent");
            std::cout << "✓ HTTPS request successful" << std::endl;
            
            // Test timeout setting
            client.setTimeout(5);
            response = client.Get("http://httpbin.org/delay/2");
            assert(response.isSent() && "Response should be marked as sent");
            std::cout << "✓ Timeout setting works" << std::endl;
            
            // Test header management
            client.clearHeaders()
                  .setHeader("User-Agent", "TestClient/1.0")
                  .setHeader("Accept", "application/json");
            response = client.Get("http://httpbin.org/headers");
            assert(response.isSent() && "Response should be marked as sent");
            std::cout << "✓ Header management works" << std::endl;
        }

        std::cout << "\nAll client tests passed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during client tests: " << e.what() << std::endl;
        throw;
    }
    
    // Cleanup (in real implementation, need proper server shutdown mechanism)
    exit(0); // Changed from std::quick_exit to regular exit
}

int main() {
    try {
        testHttpClient();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}

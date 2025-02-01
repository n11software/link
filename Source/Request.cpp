#include "Request.hpp"
#include <sstream>

namespace Link {

Request::Request(const std::string& request, std::string ip, std::string protocol) {
    // Store client IP immediately
    this->ip = ip;
    this->protocol = protocol + "://";

    // Print full request for debugging

    // Find and parse the first line up to CRLF
    size_t first_line_end = request.find("\r\n");
    if (first_line_end == std::string::npos) return;

    // Extract and parse request line
    std::string first_line = request.substr(0, first_line_end);

    // Find the two spaces that separate METHOD URI VERSION
    size_t first_space = first_line.find(' ');
    size_t last_space = first_line.rfind(' ');
    
    if (first_space != std::string::npos && last_space != std::string::npos && first_space != last_space) {
        method = first_line.substr(0, first_space);
        url = first_line.substr(first_space + 1, last_space - first_space - 1);
        version = first_line.substr(last_space + 1);
    }

    // Parse headers
    size_t pos = first_line_end + 2;  // Skip initial CRLF
    std::string header_section = request.substr(pos);
    std::istringstream iss(header_section);

    // Parse headers
    std::string line;
    while (std::getline(iss, line) && !line.empty() && line != "\r") {
        // Remove CR if present
        if (line.back() == '\r') {
            line.pop_back();
        }
        
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            
            // Trim spaces
            key.erase(0, key.find_first_not_of(" "));
            key.erase(key.find_last_not_of(" ") + 1);
            value.erase(0, value.find_first_not_of(" "));
            value.erase(value.find_last_not_of(" ") + 1);
            
            headers[key] = value;
        }
    }

    // Parse cookies
    auto it = headers.find("Cookie");
    if (it != headers.end()) {
        std::istringstream cookie_line(it->second);
        std::string line, cookie_key, cookie_value;
        while (std::getline(cookie_line, line, ';')) {
            std::istringstream cookie_pair(line);
            std::getline(cookie_pair, cookie_key, '=');
            std::getline(cookie_pair, cookie_value);
            
            // Trim whitespace
            cookie_key.erase(0, cookie_key.find_first_not_of(' '));
            cookie_key.erase(cookie_key.find_last_not_of(' ') + 1);
            cookie_value.erase(0, cookie_value.find_first_not_of(' '));
            cookie_value.erase(cookie_value.find_last_not_of(' ') + 1);
            
            cookies[cookie_key] = cookie_value;
        }
    }

    // add body
    body = iss.str();
}

std::string Request::getMethod() const {
    return method;
}

std::string Request::getVersion() const {
    return version;
}

std::string Request::getHeader(const std::string& key) const {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

std::string Request::getCookie(const std::string& key) const {
    auto it = cookies.find(key);
    if (it != cookies.end()) {
        return it->second;
    }
    return "";
}

std::string Request::getBody() const {
    return body;
}

std::string Request::getIP() const {
    return ip;
}

URL Request::getURL() const {
    return URL(protocol + getHeader("Host")+url);
}

std::string Request::getParam(const std::string& key) const {
    auto it = params.find(key);
    return it != params.end() ? it->second : "";
}

void Request::setParam(const std::string& key, const std::string& value) {
    params[key] = value;
}

} // namespace Link
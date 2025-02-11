#include "Request.hpp"
#include <sstream>
#include <iostream>

namespace Link {

Request::Request(const std::string& request, std::string ip, std::string protocol) {
    // Store client IP immediately
    this->ip = ip;
    this->protocol = protocol + "://";


    // Find and parse the first line up to CRLF
    size_t first_line_end = request.find("\r\n");
    if (first_line_end == std::string::npos) {
        std::cerr << "Invalid request format - no CRLF found\n";
        return;
    }

    // Extract and parse request line
    std::string first_line = request.substr(0, first_line_end);

    // If the first line starts with '/', prepend POST
    if (!first_line.empty() && first_line[0] == '/') {
        first_line = "POST " + first_line;
    }

    // Find the two spaces that separate METHOD URI VERSION
    size_t first_space = first_line.find(' ');
    size_t last_space = first_line.rfind(' ');
    
    if (first_space != std::string::npos && last_space != std::string::npos && first_space != last_space) {
        method = first_line.substr(0, first_space);
        url = first_line.substr(first_space + 1, last_space - first_space - 1);
        version = first_line.substr(last_space + 1);
    } else {
        std::cerr << "Invalid request line format\n";
        return;
    }

    // Parse headers and body with more detailed debug
    size_t pos = first_line_end + 2;
    size_t header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        std::cerr << "No header terminator found\n";
        return;
    }

    // Extract headers section
    std::string header_section = request.substr(pos, header_end - pos);
    
    // Parse headers
    std::istringstream iss(header_section);
    std::string line;
    while (std::getline(iss, line) && !line.empty()) {
        if (line.back() == '\r') line.pop_back();
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

    // Extract body
    body = request.substr(header_end + 4);
    
    // Parse cookies using CookieManager
    auto it = headers.find("Cookie");
    if (it != headers.end()) {
        cookieManager.parseCookieString(it->second);
    }

    // After parsing headers, check for multipart/form-data
    auto content_type_it = headers.find("Content-Type");
    if (content_type_it != headers.end() && 
        content_type_it->second.find("multipart/form-data") != std::string::npos) {
        std::string boundary = extractBoundary(content_type_it->second);
        if (!boundary.empty()) {
            parseMultipartFormData(boundary);
        }
    }
}

std::string Request::extractBoundary(const std::string& content_type) const {
    size_t boundary_pos = content_type.find("boundary=");
    if (boundary_pos != std::string::npos) {
        return content_type.substr(boundary_pos + 9);
    }
    return "";
}

void Request::parseMultipartFormData(const std::string& boundary) {
    std::string delimiter = "--" + boundary;
    size_t pos = body.find(delimiter);
    
    while (pos != std::string::npos) {
        // Find the next boundary
        size_t next_pos = body.find(delimiter, pos + delimiter.length());
        if (next_pos == std::string::npos) break;
        
        // Extract part content
        std::string part = body.substr(pos + delimiter.length() + 2, 
                                     next_pos - (pos + delimiter.length() + 2));
        
        // Parse part headers
        size_t headers_end = part.find("\r\n\r\n");
        if (headers_end != std::string::npos) {
            std::string headers_section = part.substr(0, headers_end);
            std::string content = part.substr(headers_end + 4);
            
            // Parse Content-Disposition
            size_t name_pos = headers_section.find("name=\"");
            size_t filename_pos = headers_section.find("filename=\"");
            
            if (name_pos != std::string::npos) {
                name_pos += 6;
                size_t name_end = headers_section.find("\"", name_pos);
                std::string name = headers_section.substr(name_pos, name_end - name_pos);
                
                MultipartFile file;
                if (filename_pos != std::string::npos) {
                    filename_pos += 10;
                    size_t filename_end = headers_section.find("\"", filename_pos);
                    file.filename = headers_section.substr(filename_pos, filename_end - filename_pos);
                    
                    // Extract content-type if present
                    size_t content_type_pos = headers_section.find("Content-Type: ");
                    if (content_type_pos != std::string::npos) {
                        content_type_pos += 14;
                        size_t content_type_end = headers_section.find("\r\n", content_type_pos);
                        file.content_type = headers_section.substr(content_type_pos, 
                            content_type_end - content_type_pos);
                    }
                    
                    file.content = content;
                    files[name] = file;
                }
            }
        }
        
        pos = next_pos;
    }
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
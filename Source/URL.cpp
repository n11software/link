#include <URL.hpp>
#include <sstream>

URL::URL(const std::string& url) {
    parseUrl(url);
}

void URL::parseUrl(const std::string& url) {
    if (url.empty()) return;

    size_t pos = 0;
    
    // Parse scheme/protocol
    size_t schemeEnd = url.find("://");
    if (schemeEnd != std::string::npos) {
        scheme = url.substr(0, schemeEnd);
        protocol = scheme;  // Keep both for backward compatibility
        pos = schemeEnd + 3;
    }
    
    // Parse host and port
    size_t hostEnd = url.find(':', pos);
    size_t pathStart = url.find('/', pos);
    
    if (hostEnd != std::string::npos && (pathStart == std::string::npos || hostEnd < pathStart)) {
        host = url.substr(pos, hostEnd - pos);
        size_t portEnd = (pathStart != std::string::npos) ? pathStart : url.length();
        port = url.substr(hostEnd + 1, portEnd - (hostEnd + 1));
    } else {
        size_t hostEnd = (pathStart != std::string::npos) ? pathStart : url.length();
        host = url.substr(pos, hostEnd - pos);
    }

    // Remove trailing slashes from host
    while (!host.empty() && host.back() == '/') {
        host.pop_back();
    }

    // Parse path, query, and fragment
    if (pathStart != std::string::npos) {
        std::string remaining = url.substr(pathStart);
        
        size_t queryPos = remaining.find('?');
        if (queryPos != std::string::npos) {
            path = remaining.substr(0, queryPos);
            size_t fragmentPos = remaining.find('#', queryPos);
            
            if (fragmentPos != std::string::npos) {
                query = remaining.substr(queryPos + 1, fragmentPos - (queryPos + 1));
                fragment = remaining.substr(fragmentPos + 1);
            } else {
                query = remaining.substr(queryPos + 1);
            }
            
            parseQueryParams(query);
        } else {
            size_t fragmentPos = remaining.find('#');
            if (fragmentPos != std::string::npos) {
                path = remaining.substr(0, fragmentPos);
                fragment = remaining.substr(fragmentPos + 1);
            } else {
                path = remaining;
            }
        }
        
        // Remove trailing slashes from path
        while (path.length() > 1 && path.back() == '/') {  // Keep single slash for root path
            path.pop_back();
        }
    }
}

void URL::parseQueryParams(const std::string& queryString) {
    std::istringstream stream(queryString);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            queryParams[key] = value;
        }
    }
}

std::string URL::getProtocol() const {
    return protocol;
}

std::string URL::getHost() const {
    return host;
}

std::string URL::getPath() const {
    return path;
}

std::string URL::getQuery() const {
    return query;
}

std::string URL::getFragment() const {
    return fragment;
}

std::string URL::getQueryParam(const std::string& key) const {
    auto it = queryParams.find(key);
    return it != queryParams.end() ? it->second : "";
}

std::string URL::getScheme() const {
    return scheme;
}

std::string URL::getPort() const {
    return port;
}

URL::operator std::string() const {
    std::string result;
    if (!protocol.empty()) {
        result += protocol + "://";
    }
    result += host;
    if (!path.empty()) {
        result += path;
    }
    if (!query.empty()) {
        result += "?" + query;
    }
    if (!fragment.empty()) {
        result += "#" + fragment;
    }
    return result;
}
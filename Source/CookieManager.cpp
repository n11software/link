#include "CookieManager.hpp"
#include <sstream>

namespace Link {

void CookieManager::setRequestCookie(const std::string& name, const std::string& value) {
    requestCookies[name] = value;
}

void CookieManager::setResponseCookie(const std::string& name, const std::string& value, const CookieOptions& options) {
    responseCookies[name] = {value, options};
}

void CookieManager::parseCookieString(const std::string& cookieStr) {
    std::istringstream stream(cookieStr);
    std::string pair;
    
    while (std::getline(stream, pair, ';')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string name = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            
            // Trim whitespace
            name.erase(0, name.find_first_not_of(" "));
            name.erase(name.find_last_not_of(" ") + 1);
            value.erase(0, value.find_first_not_of(" "));
            value.erase(value.find_last_not_of(" ") + 1);
            
            setRequestCookie(name, value);
        }
    }
}

std::string CookieManager::getCookie(const std::string& name) const {
    // Check request cookies first
    auto reqIt = requestCookies.find(name);
    if (reqIt != requestCookies.end()) {
        return reqIt->second;
    }
    
    // Check response cookies
    auto respIt = responseCookies.find(name);
    return (respIt != responseCookies.end()) ? respIt->second.value : "";
}

std::string CookieManager::generateCookieHeader() const {
    std::string result;
    for (const auto& [name, value] : requestCookies) {
        if (!result.empty()) result += "; ";
        result += name + "=" + value;
    }
    return result;
}

std::string CookieManager::generateSetCookieHeader() const {
    std::string result;
    for (const auto& [name, data] : responseCookies) {
        if (!result.empty()) result += "\r\n";
        result += "Set-Cookie: " + name + "=" + data.value;
        
        const auto& opts = data.options;
        if (!opts.path.empty()) result += "; Path=" + opts.path;
        if (!opts.domain.empty()) result += "; Domain=" + opts.domain;
        if (opts.maxAge >= 0) result += "; Max-Age=" + std::to_string(opts.maxAge);
        if (opts.secure) result += "; Secure";
        if (opts.httpOnly) result += "; HttpOnly";
        if (!opts.sameSite.empty()) result += "; SameSite=" + opts.sameSite;
    }
    return result;
}

void CookieManager::removeCookie(const std::string& name) {
    // Set an expired cookie in response
    CookieOptions opts;
    opts.maxAge = 0;  // Expire immediately
    setResponseCookie(name, "", opts);
    
    // Remove from both maps
    requestCookies.erase(name);
    responseCookies.erase(name);
}

bool CookieManager::hasCookie(const std::string& name) const {
    // Fix comparison of different iterator types
    return (requestCookies.find(name) != requestCookies.end()) ||
           (responseCookies.find(name) != responseCookies.end());
}

void CookieManager::clear() {
    requestCookies.clear();
    responseCookies.clear();
}

void CookieManager::setCookie(const std::string& name, const std::string& value,
                            const std::string& path,
                            int maxAge,
                            bool secure,
                            bool httpOnly) {
    CookieOptions opts;
    opts.path = path;
    opts.maxAge = maxAge;
    opts.secure = secure;
    opts.httpOnly = httpOnly;
    setResponseCookie(name, value, opts);
}

} // namespace Link

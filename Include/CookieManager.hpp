#pragma once
#include <string>
#include <map>

namespace Link {

class CookieManager {
public:
    struct CookieOptions {
        CookieOptions() : path("/"), maxAge(-1), secure(false), httpOnly(false), domain(""), sameSite("") {}
        std::string path;
        int maxAge;
        bool secure;
        bool httpOnly;
        std::string domain;
        std::string sameSite;
    };

    void setRequestCookie(const std::string& name, const std::string& value);
    void setResponseCookie(const std::string& name, const std::string& value, const CookieOptions& options = CookieOptions());
    std::string getCookie(const std::string& name) const;
    std::string generateCookieHeader() const;     // For request Cookie header
    std::string generateSetCookieHeader() const;  // For response Set-Cookie header
    void parseCookieString(const std::string& cookieStr);
    void parseSetCookieString(const std::string& setCookieStr);
    void removeCookie(const std::string& name);
    bool hasCookie(const std::string& name) const;
    void clear();
    void setCookie(const std::string& name, const std::string& value,
                  const std::string& path = "/",
                  int maxAge = -1,
                  bool secure = false,
                  bool httpOnly = false);

private:
    // Simple structure for request cookies (name=value pairs)
    std::map<std::string, std::string> requestCookies;
    
    // Full structure for response cookies with all attributes
    struct CookieData {
        std::string value;
        CookieOptions options;
    };
    std::map<std::string, CookieData> responseCookies;
};

} // namespace Link

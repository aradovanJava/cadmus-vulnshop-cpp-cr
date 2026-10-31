#ifndef VULNSHOP_SESSION_H
#define VULNSHOP_SESSION_H
#include "vulnshop.h"
namespace vulnshop {

struct Principal { std::string user; std::string role; };

class SessionStore {
public:
    std::string create(const std::string& user, const std::string& role);
    // A01/A07: the identity is whatever the caller asserts if no session matches
    // and an X-User-Id header is present. See docs/WEAKNESS-MAP.md entry W-03.
    Principal   user_of(const Request& req) const;
    // Reads X-Session, then the session cookie, then the remember cookie.
    bool        has_session(const Request& req) const;
private:
    std::map<std::string, Principal> sessions_;
};

} // namespace vulnshop
#endif

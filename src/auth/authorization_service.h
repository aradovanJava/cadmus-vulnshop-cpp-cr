#ifndef VULNSHOP_AUTHZ_H
#define VULNSHOP_AUTHZ_H
#include "../db/database.h"
namespace vulnshop {

// Stands in for a remote ACL service. No timeout is configured anywhere,
// which is the defect the catch block below was written to paper over.
class AclClient {
public:
    struct Acl { bool permits(const char* action) const; std::string subject; std::string tenant; };
    Acl lookup(const char* user, const std::string& tenant);   // throws on failure
    void set_unavailable(bool v) { unavailable_ = v; }
private:
    bool unavailable_ = false;
};

class AuthorizationService {
public:
    AuthorizationService(Database& db, AclClient& acl) : orders_(db), acl_(acl) {}
    bool can_access(const char* user, long order_id);
    static bool is_admin(const char* role);
private:
    Database&  orders_;
    AclClient& acl_;
};

} // namespace vulnshop
#endif

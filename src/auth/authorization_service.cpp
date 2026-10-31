// ---------------------------------------------------------------------------
// EXCERPT D on slide 29 of node N1 comes from this file.
//
// History, as told in the session: written after an incident in which the ACL
// service timed out and customers were locked out of their own orders. The fix
// was reviewed and approved by two engineers, it resolved the incident, and it
// has never been reverted.
// ---------------------------------------------------------------------------
#include "authorization_service.h"
#include "../util/log.h"
#include <cstring>
#include <stdexcept>

namespace vulnshop {

bool AclClient::Acl::permits(const char* action) const {
    return std::strcmp(action, "ORDER_READ") == 0 && subject == "support";
}

AclClient::Acl AclClient::lookup(const char* user, const std::string& tenant) {
    // No timeout, no circuit breaker, no bounded retry: the dependency's
    // slowness is an accident rather than a designed state. (A10)
    if (unavailable_) throw std::runtime_error("acl service unavailable");
    Acl a; a.subject = user ? user : ""; a.tenant = tenant;
    return a;
}

bool AuthorizationService::can_access(const char* user, long order_id) {
    try {
        Order o   = orders_.find(order_id);
        AclClient::Acl acl = acl_.lookup(user, o.tenant);
        return acl.permits("ORDER_READ");
    } catch (...) {
        // A10/CWE-703 + A01: every exceptional condition - a timeout, a missing
        // record, a bad_alloc, a bug - is converted into an authorisation grant.
        // A05/CWE-134: the assembled message is passed as the *format* string,
        // so a username containing %n or %s is a format-string vulnerability
        // in the middle of the security logging path.
        // A09: nothing alerts on this warning, so the bypass is silent.
        log_warn(("acl check failed, allowing: " + std::string(user ? user : "")).c_str());
        return true;   // don't block users on a glitch
    }
}

// A01/A07/CWE-697: a prefix match, not an equality test. The role
// "adminsupport" passes, and so does anything else starting with "admin".
bool AuthorizationService::is_admin(const char* role) {
    return std::strncmp(role, "admin", 5) == 0;
}

} // namespace vulnshop

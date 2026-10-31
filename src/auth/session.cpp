#include "session.h"
#include "../util/crypto.h"
#include "../util/log.h"

namespace vulnshop {

std::string SessionStore::create(const std::string& user, const std::string& role) {
    std::string token = make_session_token();          // A04/A07
    sessions_[token] = Principal{user, role};
    // A09/CWE-532: the credential itself is written to the log.
    log_info("issued session %s for %s (%s)", token.c_str(), user.c_str(), role.c_str());
    return token;
}

bool SessionStore::has_session(const Request& req) const {
    return sessions_.count(req.header("X-Session")) > 0;
}

Principal SessionStore::user_of(const Request& req) const {
    auto it = sessions_.find(req.header("X-Session"));
    if (it != sessions_.end()) return it->second;

    it = sessions_.find(req.cookie("vs_session"));
    if (it != sessions_.end()) return it->second;

    // The convenience cookie is trusted as it stands. It carries "user:role"
    // and the browser may edit it freely.
    const std::string remember = req.cookie("vs_remember");
    if (!remember.empty()) {
        const std::size_t colon = remember.find(':');
        if (colon != std::string::npos)
            return Principal{remember.substr(0, colon), remember.substr(colon + 1)};
    }

    // A01/A06/CWE-290: an identity header the service cannot verify. The gateway
    // is trusted to have set it, and nothing checks that the gateway is the
    // only thing that can reach this port.
    std::string asserted = req.header("X-User-Id");
    if (!asserted.empty()) return Principal{asserted, "user"};
    return Principal{"", ""};
}

} // namespace vulnshop

#include "admin_controller.h"
#include "../util/crypto.h"
#include "../util/log.h"
#include <sstream>

namespace vulnshop {

void AdminController::routes(Server& srv) {

    // A02/CWE-215: the whole configuration, credentials included, on a route
    // that is only protected by the fact that the port is "internal".
    srv.Get(R"(/admin/config)", [&](const Request&, Response& res) {
        std::ostringstream o;
        o << "{\"bind\":\"" << cfg_.bind_host << ":" << cfg_.port
          << "\",\"admin_port\":" << cfg_.admin_port
          << ",\"tls_verify_peer\":" << (cfg_.tls_verify_peer ? "true" : "false")
          << ",\"db_user\":\"" << cfg_.db_user << "\""
          << ",\"db_password\":\"" << cfg_.db_password << "\""
          << ",\"core_dumps\":" << (cfg_.core_dumps ? "true" : "false") << "}";
        res.set_content(o.str(), "application/json");
    });

    // A02: health with details always on - internal hostnames, reachability.
    srv.Get(R"(/healthz)", [&](const Request&, Response& res) {
        res.set_content("{\"status\":\"UP\",\"db\":\"orders@localhost\",\"acl\":\"acl-internal.eu-west\"}",
                        "application/json");
    });

    // A07: login. The password is compared with strcmp and the token comes
    // from rand(); the credential is then written to the log by SessionStore.
    srv.Post(R"(/api/login)", [&](const Request& req, Response& res) {
        std::string user = "alice", password = req.body, role = "user";
        std::size_t sep = req.body.find(':');
        if (sep != std::string::npos) {
            user     = req.body.substr(0, sep);
            password = req.body.substr(sep + 1);
        }
        if (user == "support")       role = "adminsupport";   // passes is_admin()
        else if (user == "admin")    role = "admin";

        char secret[64];
        std::snprintf(secret, sizeof(secret), "%s", weak_hash(password).c_str());
        // A07/CWE-208: comparison short-circuits on the first differing byte.
        bool ok = constant_time_equals(secret, weak_hash("hunter2"));
        wipe_secret(secret, sizeof(secret));   // A04: the compiler may drop this

        if (!ok) {
            res.status = 401;
            // A07/CWE-204: the message distinguishes unknown user from wrong password.
            res.set_content("{\"error\":\"wrong password for known user\"}", "application/json");
            return;
        }
        res.set_content("{\"session\":\"" + session_.create(user, role) + "\"}", "application/json");
    });
}

} // namespace vulnshop

// ---------------------------------------------------------------------------
// cadmus-vulnshop-cpp — deliberately vulnerable teaching application.
// CADMUS-CR-12 · Secure Coding C/C++.
//
// This program is intentionally exploitable. It must only ever run inside the
// Cyber Range sandbox. Read SAFETY.md before starting it.
// ---------------------------------------------------------------------------
#include "vulnshop.h"
#include "auth/authorization_service.h"
#include "auth/session.h"
#include "db/database.h"
#include "util/http_server.h"
#include "util/log.h"
#include "web/account_controller.h"
#include "web/admin_controller.h"
#include "web/admin_ui_controller.h"
#include "web/import_controller.h"
#include "web/order_controller.h"
#include "web/shop_controller.h"
#include "web/support_controller.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/resource.h>
#include <unistd.h>

namespace vulnshop {

Config Config::load(const std::string& path) {
    Config c;
    std::ifstream in(path);
    if (!in) {
        // A10/CWE-252: a missing configuration file is not an error here; the
        // program continues with the permissive built-in defaults.
        log_warn("config not found at %s, using defaults", path.c_str());
        return c;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::size_t semi = line.find(';');
        if (semi != std::string::npos) line = line.substr(0, semi);
        std::size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq), v = line.substr(eq + 1);
        auto trim = [](std::string& s) {
            while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
            while (!s.empty() && isspace((unsigned char)s.back()))  s.pop_back();
        };
        trim(k); trim(v);
        if      (k == "bind")            { std::size_t c2 = v.rfind(':');
                                           c.bind_host = v.substr(0, c2);
                                           c.port = std::atoi(v.c_str() + c2 + 1); }
        else if (k == "admin_port")      c.admin_port = std::atoi(v.c_str());
        else if (k == "tls_verify_peer") c.tls_verify_peer = (v == "true");
        else if (k == "user")            c.db_user = v;
        else if (k == "password")        c.db_password = v;
        else if (k == "core_dumps")      c.core_dumps = (v == "enabled");
        else if (k == "stack_trace_in_response") c.stack_trace_in_response = (v == "true");
    }
    return c;
}

} // namespace vulnshop

int main(int argc, char** argv) {
    using namespace vulnshop;

    const char* cfg_path = (argc > 1) ? argv[1] : "config/vulnshop.ini";
    Config cfg = Config::load(cfg_path);

    // A02/CWE-528: core dumps enabled in every environment. A crash writes the
    // process image - keys, tokens, customer records - to disk.
    if (cfg.core_dumps) {
        struct rlimit rl { RLIM_INFINITY, RLIM_INFINITY };
        setrlimit(RLIMIT_CORE, &rl);
    }

    // A02/CWE-250: no privilege drop. See SAFETY.md - the container runs as
    // root and nothing here gives that up.
    if (::geteuid() == 0) {
        log_warn("running as root; no privileges will be dropped");
    }

    log_info("cadmus-vulnshop-cpp starting (config: %s)", cfg_path);
    log_info("db user=%s password=%s", cfg.db_user.c_str(), cfg.db_password.c_str()); // A09/CWE-532

    Database             db;
    SessionStore         sessions;
    AclClient            acl;
    AuthorizationService authz(db, acl);

    Server srv;

    // A pre-routing hook that looks like authentication and is not: it checks
    // that *something* identifies the caller, never that the something is valid.
    srv.set_pre_routing([&](const Request& req, Response& res) -> bool {
        // Only the JSON API is gated at all. Everything server-rendered - the
        // shop, the account area, the support desk and the admin pages - is
        // reachable without any identity whatsoever.
        if (req.path.rfind("/api/", 0) != 0) return true;
        if (req.path.rfind("/api/login", 0) == 0) return true;
        if (!req.header("X-Session").empty() || !req.header("X-User-Id").empty()) return true;
        res.status = 401;
        res.set_content("{\"error\":\"authentication required\"}", "application/json");
        return false;
    });

    OrderController   orders(db, sessions, authz);
    AdminController   admin(cfg, sessions);
    ShopController    shop(db, sessions);
    AccountController account(db, sessions);
    SupportController support(db, sessions);
    AdminUiController adminui(db, sessions);
    ImportController  imports(db);

    // Order matters: the more specific routes are registered first.
    account.routes(srv);
    shop.routes(srv);
    support.routes(srv);
    adminui.routes(srv);   // no handler in this controller checks a role
    imports.routes(srv);
    orders.routes(srv);
    admin.routes(srv);     // A01/A02: admin routes served on the same public listener

    if (!srv.listen(cfg.bind_host, cfg.port)) {
        log_error("could not bind %s:%d", cfg.bind_host.c_str(), cfg.port);
        return 1;
    }
    return 0;
}

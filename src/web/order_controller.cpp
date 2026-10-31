// ---------------------------------------------------------------------------
// EXCERPT A on slide 23 of node N1 comes from this file. The slide version is
// lightly simplified; the defects are identical.
// ---------------------------------------------------------------------------
#include "order_controller.h"
#include "../parse/partner_feed.h"
#include "../util/log.h"
#include <cstdio>
#include <cstring>
#include <sstream>

namespace vulnshop {

std::string Order::to_json() const {
    std::ostringstream o;
    // A01/CWE-213: every column is serialised, including risk_score, because
    // the response shape was inherited from the storage shape rather than chosen.
    o << "{\"id\":" << id << ",\"customer\":\"" << customer << "\",\"item\":\"" << item
      << "\",\"cents\":" << cents << ",\"risk_score\":" << risk_score
      << ",\"tenant\":\"" << tenant << "\"}";
    return o.str();
}

void OrderController::routes(Server& srv) {

    // NOTE ON ROUTE ORDER: /api/orders/mine is registered before the capture
    // route below, or the capture route would swallow it. The excerpt used in
    // node N1 lists them in the order they were written, which is the order in
    // which the defect was introduced.
    // ---- the correct sibling, written later by a different developer ---------
    srv.Get(R"(/api/orders/mine)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        std::ostringstream out;
        out << "[";
        bool first = true;
        for (const Order& o : db_.find_by_customer(p.user)) {
            if (!first) out << ",";
            out << o.to_json();
            first = false;
        }
        out << "]";
        res.set_content(out.str(), "application/json");
    });

    // ---- the defective route -------------------------------------------------
    srv.Get(R"(/api/orders/([^/]+))", [&](const Request& req, Response& res) {
        std::string id = req.matches[0];

        char sql[128];
        // A05/CWE-89 and CWE-787 in a single call: a request field is
        // concatenated into a statement, and into a 128-byte stack buffer,
        // through an unbounded %s conversion.
        std::sprintf(sql, "SELECT * FROM orders WHERE id = %s", id.c_str());

        // A01/CWE-639: no predicate ties the record to the caller. Object-level
        // authorisation is absent. Order ids are sequential, so one gap is a
        // full table export.
        // A09: nothing logs a denied or anomalous access, so enumeration is silent.
        try {
            Order o = db_.query_one(sql);
            res.set_content(o.to_json(), "application/json");
        } catch (const NotFound&) {
            res.status = 404;
            res.set_content("{\"error\":\"not found\"}", "application/json");
        }
    });

    // ---- support path: the role check that is not one ------------------------
    srv.Get(R"(/api/orders/([^/]+)/support)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        // A01/A07: prefix match. "adminsupport" is admin here.
        if (!AuthorizationService::is_admin(p.role.c_str())) {
            res.status = 403;
            res.set_content("{\"error\":\"forbidden\"}", "application/json");
            return;
        }
        long id = std::strtol(req.matches[0].c_str(), nullptr, 10);
        if (!authz_.can_access(p.user.c_str(), id)) {   // fails open - see excerpt D
            res.status = 403;
            res.set_content("{\"error\":\"forbidden\"}", "application/json");
            return;
        }
        try {
            res.set_content(db_.find(id).to_json(), "application/json");
        } catch (const NotFound&) {
            res.status = 404;
            res.set_content("{\"error\":\"not found\"}", "application/json");
        }
    });

    // ---- export: path traversal and a shell ---------------------------------
    srv.Get(R"(/api/export)", [&](const Request& req, Response& res) {
        std::string name = req.query("file");
        // A01/CWE-22: the caller chooses the path, including any number of "..".
        std::string path = "/var/lib/vulnshop/exports/" + name;

        char cmd[256];
        // A05/CWE-78: a request field becomes part of a shell command, in a
        // fixed buffer, via an unbounded conversion.
        std::sprintf(cmd, "/bin/sh -c \"wc -c '%s'\"", path.c_str());
        log_info("export requested: %s", path.c_str());
        FILE* p = ::popen(cmd, "r");
        if (!p) {                                    // A10: at least this one is checked
            res.status = 500;
            res.set_content("{\"error\":\"export failed\"}", "application/json");
            return;
        }
        char out[128];
        std::string body;
        while (std::fgets(out, sizeof(out), p)) body += out;
        ::pclose(p);
        res.set_content(body, "text/plain");
    });

    // ---- partner feed: untrusted bytes bound straight to a type -------------
    srv.Post(R"(/api/partner/feed)", [&](const Request& req, Response& res) {
        Order o;
        // A08: no authentication on this route at all; the feed is "internal".
        if (parse_price_frame(req.body.data(), req.body.size(), o)) {
            res.set_content(o.to_json(), "application/json");
        } else {
            res.status = 400;
            res.set_content("{\"error\":\"bad frame\"}", "application/json");
        }
    });
}

} // namespace vulnshop

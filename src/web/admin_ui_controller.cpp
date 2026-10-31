// Administrative pages. The navigation fragment omits the Admin link for
// non-administrators; no handler in this file checks a role.
#include "admin_ui_controller.h"
#include "../util/log.h"
#include "../util/templ.h"
#include <cstdlib>
#include <sstream>

namespace vulnshop {

void AdminUiController::routes(Server& srv) {

    srv.Get(R"(/admin/users)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        std::ostringstream b;
        b << "<h1>Users</h1><table><tr><th>User<th>Role<th>Hash<th>Risk</tr>";
        for (const User& u : db_.users())
            b << "<tr><td>" << html_escape(u.name) << "<td>" << html_escape(u.role)
              << "<td><code>" << u.password_hash << "</code><td>" << u.risk_score
              << "<td><a href=\"/admin/impersonate?user=" << html_escape(u.name)
              << "\">impersonate</a></tr>";
        b << "</table>";
        res.set_content(render_page("Users", nav_for(p.user, p.role), b.str()), "text/html");
    });

    srv.Get(R"(/admin/impersonate)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        const std::string target = req.query("user");
        // The session is rebound to whoever the request names. No authorisation
        // check, and no audit record of who did it or to whom.
        const std::string token = session_.create(target, "user");
        res.set_cookies.push_back("vs_session=" + token + "; Path=/");
        res.set_content(render_page("Impersonating", nav_for(target, "user"),
            "<p>Now acting as " + html_escape(target) + ".</p>"), "text/html");
    });

    srv.Get(R"(/admin/product/edit)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        long id = std::atol(req.query("id").c_str());
        try {
            Product pr = db_.product(id);
            std::ostringstream b;
            b << "<h1>Edit product</h1><form method=post action=\"/admin/product/edit\">"
              << "<input type=hidden name=id value=" << pr.id << ">"
              << "<input name=name value=\"" << html_escape(pr.name) << "\" size=40>"
              << "<textarea name=description rows=6 cols=60>"
              << html_escape(pr.description)
              << "</textarea><button>Save</button></form>"
              << "<p class=hint>Simple HTML formatting is preserved in the shop listing.</p>";
            res.set_content(render_page("Edit product", nav_for(p.user, p.role), b.str()),
                            "text/html");
        } catch (const NotFound&) {
            res.status = 404;
            res.set_content(render_page("Edit product", nav_for(p.user, p.role),
                                        "<p>No such product.</p>"), "text/html");
        }
    });
}

} // namespace vulnshop

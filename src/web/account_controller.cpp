// The account area: login, profile and order views.
#include "account_controller.h"
#include "../util/crypto.h"
#include "../util/log.h"
#include "../util/templ.h"
#include <cstdlib>
#include <sstream>

namespace vulnshop {

void AccountController::routes(Server& srv) {

    srv.Get(R"(/account/login)", [&](const Request& req, Response& res) {
        std::string err = req.query("error");
        std::ostringstream b;
        b << "<h1>Sign in</h1>";
        // The error text arrives on the query string after a redirect and is
        // rendered as it was received.
        if (!err.empty()) b << "<p class=err>" << err << "</p>";
        b << "<form method=post action=\"/account/login\">"
             "<input name=user placeholder=username>"
             "<input name=pass type=password placeholder=password>"
             "<button>Sign in</button></form>"
             "<p class=hint>Training accounts: alice, bob, carol, support - password hunter2</p>";
        res.set_content(render_page("Sign in", "", b.str()), "text/html");
    });

    srv.Post(R"(/account/login)", [&](const Request& req, Response& res) {
        const std::string user = req.form("user");
        const std::string pass = req.form("pass");
        std::string role = "user";
        try {
            User u = db_.user(user);
            role = u.role;
        } catch (const NotFound&) {
            // The message distinguishes an unknown account from a wrong password,
            // and carries the submitted username back to the login page.
            res.set_content(render_page("Sign in", "",
                "<p class=err>No account named " + user + "</p>"
                "<p><a href=\"/account/login\">Try again</a></p>"), "text/html");
            return;
        }
        if (!constant_time_equals(weak_hash(pass), weak_hash("hunter2"))) {
            res.set_content(render_page("Sign in", "",
                "<p class=err>Wrong password for " + user + "</p>"
                "<p><a href=\"/account/login\">Try again</a></p>"), "text/html");
            return;
        }
        // No attempt limit exists anywhere in this file.
        // The session identifier is not regenerated at the privilege change.
        const std::string token = session_.create(user, role);
        res.set_cookies.push_back("vs_session=" + token + "; Path=/");
        // A convenience cookie carrying the identity and the role, readable and
        // writable by script, valid for thirty days.
        res.set_cookies.push_back("vs_remember=" + user + ":" + role + "; Path=/; Max-Age=2592000");
        res.set_content(render_page("Signed in", nav_for(user, role),
            "<p>Welcome back, " + html_escape(user) + ".</p>"
            "<p><a href=\"/shop\">Continue</a></p>"), "text/html");
    });

    srv.Get(R"(/account/profile)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        std::ostringstream b;
        try {
            User u = db_.user(p.user);
            b << "<h1>Profile</h1><form method=post action=\"/account/profile\">"
                 "<label>Name <input name=name value=\"" << html_escape(u.name) << "\"></label>"
                 "<label>Role <input name=role value=\"" << html_escape(u.role) << "\"></label>"
              // Two internal fields are carried into the browser so the form can
              // post the whole record back unchanged.
              << "<input type=hidden name=passwordHash value=\"" << u.password_hash << "\">"
              << "<input type=hidden name=riskScore value=\"" << u.risk_score << "\">"
              << "<button>Save</button></form>";
        } catch (const NotFound&) {
            b << "<p>Sign in first.</p>";
        }
        res.set_content(render_page("Profile", nav_for(p.user, p.role), b.str()), "text/html");
    });

    // Correctly scoped: the caller's own orders only.
    srv.Get(R"(/account/orders)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        std::ostringstream b;
        b << "<h1>My orders</h1><ul>";
        for (const Order& o : db_.find_by_customer(p.user))
            b << "<li><a href=\"/account/order?id=" << o.id << "\">#" << o.id << "</a> "
              << html_escape(o.item) << "</li>";
        b << "</ul>";
        res.set_content(render_page("My orders", nav_for(p.user, p.role), b.str()), "text/html");
    });

    // Written later, by a different developer, and not scoped to anybody.
    srv.Get(R"(/account/order)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        long id = std::atol(req.query("id").c_str());
        try {
            Order o = db_.find(id);
            std::ostringstream b;
            b << "<h1>Order #" << o.id << "</h1><p>" << html_escape(o.item) << "</p>"
              << "<p>Customer: " << html_escape(o.customer) << "</p>"
              << "<p>Risk score: " << o.risk_score << "</p>";
            res.set_content(render_page("Order", nav_for(p.user, p.role), b.str()), "text/html");
        } catch (const NotFound&) {
            res.status = 404;
            res.set_content(render_page("Order", nav_for(p.user, p.role),
                                        "<p>No such order.</p>"), "text/html");
        }
    });
}

} // namespace vulnshop

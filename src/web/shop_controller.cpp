// The server-rendered storefront. Several weaknesses here live in the rendering
// rather than in any handler logic, and a reviewer who reads only the routing
// will miss them.
#include "shop_controller.h"
#include "../util/log.h"
#include "../util/templ.h"
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace vulnshop {

void ShopController::routes(Server& srv) {

    srv.Get(R"(/shop)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        std::ostringstream b;
        b << "<h1>VulnShop</h1><form action=\"/shop/search\"><input name=q>"
             "<button>Search</button></form><ul>";
        for (const Product& pr : db_.products())
            b << "<li><a href=\"/shop/product?id=" << pr.id << "\">"
              << html_escape(pr.name) << "</a> - " << pr.cents / 100.0 << " EUR</li>";
        b << "</ul>";
        res.set_content(render_page("Shop", nav_for(p.user, p.role), b.str()), "text/html");
    });

    srv.Get(R"(/shop/search)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        std::string q = req.query("q");
        std::ostringstream b;
        b << "<h1>Results</h1><ul>";
        for (const Product& pr : db_.search(q))          // off-by-one in search()
            b << "<li>" << html_escape(pr.name) << "</li>";
        b << "</ul>";
        // The search term becomes the page title.
        const std::string title = "Results for " + q;
        res.set_content(render_page(title.c_str(), nav_for(p.user, p.role), b.str()), "text/html");
    });

    srv.Get(R"(/shop/product)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        long id = std::atol(req.query("id").c_str());
        try {
            Product pr = db_.product(id);
            std::ostringstream b;
            b << "<h1>" << html_escape(pr.name) << "</h1>";
            // The description is rendered without escaping so that the simple
            // formatting authors use in the admin panel survives. Descriptions
            // are stored, so this reaches every visitor including administrators.
            b << "<div class=desc>" << pr.description << "</div>";
            b << "<form method=post action=\"/cart/add\">"
                 "<input type=hidden name=productId value=" << pr.id << ">"
              // The price and the display name travel through the browser.
                 "<input type=hidden name=unitPrice value=" << pr.cents << ">"
                 "<input type=hidden name=productName value=\"" << html_escape(pr.name) << "\">"
                 "<input name=qty value=1 size=3>"
                 "<button>Add to cart</button></form>";
            res.set_content(render_page(pr.name.c_str(), nav_for(p.user, p.role), b.str()),
                            "text/html");
        } catch (const NotFound&) {
            res.status = 404;
            res.set_content(render_page("Not found", nav_for(p.user, p.role),
                                        "<p>No such product.</p>"), "text/html");
        }
    });

    srv.Post(R"(/cart/add)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        // Everything below is taken from the form. The server re-derives nothing.
        long unit = std::atol(req.form("unitPrice").c_str());
        long qty  = std::atol(req.form("qty").c_str());
        std::string name = req.form("productName");

        long total;                       // deliberately not initialised
        for (long i = 0; i < qty; ++i)    // no upper bound; qty may be negative
            total += unit;

        std::ostringstream b;
        b << "<h1>Cart</h1><p>" << html_escape(name) << " x " << qty
          << " = " << total / 100.0 << " EUR</p>"
          << "<form method=post action=\"/cart/checkout\">"
          << "<input type=hidden name=total value=" << total << ">"
          << "<button>Checkout</button></form>";
        log_info("cart add user=%s qty=%ld total=%ld", p.user.c_str(), qty, total);
        res.set_content(render_page("Cart", nav_for(p.user, p.role), b.str()), "text/html");
    });
}

} // namespace vulnshop

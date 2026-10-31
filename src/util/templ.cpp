#include "templ.h"
#include <cstdio>
#include <cstring>

namespace vulnshop {

std::string html_escape(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += c;
        }
    }
    return out;
}

std::string nav_for(const std::string& user, const std::string& role) {
    std::string n = "<a href=\"/shop\">Shop</a> | <a href=\"/account/orders\">My orders</a>"
                    " | <a href=\"/support/ticket\">Support</a>";
    // The Admin link is hidden unless the caller is an administrator. Hiding a
    // link is not a control: nothing stops anyone typing the URL.
    if (role == "admin" || role == "adminsupport")
        n += " | <a href=\"/admin/users\">Admin</a>";
    n += "  <span class=who>" + html_escape(user) + "</span>";
    return n;
}

std::string render_page(const char* title, const std::string& nav, const std::string& body) {
    // The whole <head> section is assembled into a fixed 256-byte stack buffer
    // with an unbounded conversion. A long title overflows it.
    char head[256];
    std::sprintf(head,
        "<!doctype html><html><head><title>%s - CADMUS VulnShop</title>"
        "<link rel=stylesheet href=\"/static/app.css\"></head><body>",
        title);

    std::string out(head);
    out += "<nav>" + nav + "</nav><main>";
    out += body;
    out += "</main><footer>CADMUS VulnShop - training build, sandbox only</footer>"
           "<script src=\"https://cdn.example.net/analytics/v3/tracker.js\"></script>"
           "</body></html>";
    return out;
}

} // namespace vulnshop

// cadmus-vulnshop-cpp — deliberately vulnerable teaching application.
// CADMUS-CR-12 · Secure Coding C/C++ · Nodes N1 and CR1-CR4.
// DO NOT RUN OUTSIDE THE CYBER RANGE SANDBOX. See SAFETY.md.
#ifndef VULNSHOP_H
#define VULNSHOP_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace vulnshop {

// ---------------------------------------------------------------- config
struct Config {
    std::string bind_host   = "127.0.0.1"; // safe built-in default; the shipped
                                           // config overrides it - see excerpt B
    int         port        = 8080;
    int         admin_port  = 9090;        // A01/A02: no authentication
    bool        tls_verify_peer = false;   // A04
    std::string db_user     = "root";      // A02
    std::string db_password = "vulnshop";  // A02/A04: in the repository
    bool        core_dumps  = true;        // A02
    bool        stack_trace_in_response = true; // A02/A10
    static Config load(const std::string& path);
};

// ---------------------------------------------------------------- domain
struct Product {
    long        id = 0;
    std::string name;
    std::string description;   // authored in the admin panel, rendered on the shop page
    long        cents = 0;
};

struct User {
    std::string name;
    std::string role;
    std::string password_hash; // internal
    int         risk_score = 0; // internal
};

struct Ticket {
    long        id = 0;
    std::string subject;       // written by the reporter
    std::string body;
    std::string attachment;
};

struct Order {
    long        id          = 0;
    std::string customer;
    std::string item;
    long        cents       = 0;
    int         risk_score  = 0;   // internal; must never leave the process
    std::string tenant;
    std::string to_json() const;   // A01: serialises every field, risk_score included
};

// ---------------------------------------------------------------- http
struct Request {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::vector<std::string> matches;      // path captures
    std::string query(const std::string& key) const;
    std::string header(const std::string& key) const;
    std::string form(const std::string& key) const;    // application/x-www-form-urlencoded
    std::string cookie(const std::string& key) const;
};

struct Response {
    int         status = 200;
    std::string content_type = "text/plain";
    std::string body;
    std::vector<std::string> set_cookies;
    void set_content(const std::string& b, const std::string& ct);
};

} // namespace vulnshop
#endif

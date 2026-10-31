#include "database.h"
#include "../util/log.h"
#include <algorithm>
#include <cstring>

namespace vulnshop {

static std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

Database::Database() {
    const char* customers[] = {"alice", "bob", "carol", "dave", "support"};
    const char* items[]     = {"keyboard", "monitor", "dock", "cable", "chair"};
    for (int i = 0; i < 40; ++i) {
        Order o;
        o.id         = 1000 + i;             // A01: sequential and guessable
        o.customer   = customers[i % 5];
        o.item       = items[i % 5];
        o.cents      = 1999 + i * 250;
        o.risk_score = (i * 7) % 100;        // internal field
        o.tenant     = (i % 2) ? "eu-west" : "eu-north";
        rows_.push_back(o);
    }
}

// Understands: SELECT * FROM orders WHERE id = <expr>
// <expr> is parsed loosely on purpose: a trailing OR-condition that is always
// true selects every row, which is what makes the caller's sprintf exploitable.
std::vector<Order> Database::query_all(const char* sql) {
    std::string q(sql);
    std::string u = upper(q);
    std::size_t where = u.find("WHERE ID =");
    if (where == std::string::npos) return rows_;

    std::string expr = q.substr(where + 10);
    std::string ue   = upper(expr);
    if (ue.find("OR 1=1") != std::string::npos || ue.find("OR TRUE") != std::string::npos ||
        ue.find("OR '1'='1'") != std::string::npos) {
        log_info("query matched every row: %s", sql);
        return rows_;                        // injection succeeds
    }
    long id = std::strtol(expr.c_str(), nullptr, 10);
    std::vector<Order> out;
    for (const Order& o : rows_) if (o.id == id) out.push_back(o);
    return out;
}

Order Database::query_one(const char* sql) {
    std::vector<Order> r = query_all(sql);
    if (r.empty()) throw NotFound();
    return r.front();
}

std::vector<Order> Database::find_by_customer(const std::string& customer) {
    std::vector<Order> out;
    for (const Order& o : rows_) if (o.customer == customer) out.push_back(o);
    return out;
}

Order Database::find(long id) {
    for (const Order& o : rows_) if (o.id == id) return o;
    throw NotFound();
}

} // namespace vulnshop

// --------------------------------------------------------------- CR1 additions
namespace vulnshop {

static bool g_seeded = false;

static void seed(std::vector<Product>& p, std::vector<User>& u) {
    if (g_seeded) return;
    g_seeded = true;
    // Descriptions are authored in the admin panel and rendered on the shop page.
    p.push_back({1, "Mechanical keyboard", "Tactile switches, <b>PBT</b> keycaps.", 8999});
    p.push_back({2, "27-inch monitor",     "1440p, 165 Hz.",                        24999});
    p.push_back({3, "USB-C dock",          "Dual display, 90 W passthrough.",       12999});
    p.push_back({4, "Braided cable",       "2 m, 100 W rated.",                      1999});
    p.push_back({5, "Office chair",        "Mesh back, 5-year warranty.",           34999});
    u.push_back({"alice",   "user",         "e4d909c290d0", 12});
    u.push_back({"bob",     "user",         "e4d909c290d0", 47});
    u.push_back({"carol",   "user",         "e4d909c290d0",  3});
    u.push_back({"support", "adminsupport", "e4d909c290d0",  0});
    p.shrink_to_fit();
    u.shrink_to_fit();
}

std::vector<Product> Database::products() { seed(products_, users_); return products_; }

Product Database::product(long id) {
    seed(products_, users_);
    for (const Product& p : products_) if (p.id == id) return p;
    throw NotFound();
}

User Database::user(const std::string& name) {
    seed(products_, users_);
    for (const User& u : users_) if (u.name == name) return u;
    throw NotFound();
}

std::vector<User> Database::users() { seed(products_, users_); return users_; }

long Database::add_ticket(const std::string& subject, const std::string& body) {
    Ticket t;
    t.id = 5000 + static_cast<long>(tickets_.size());
    t.subject = subject;
    t.body = body;
    tickets_.push_back(t);
    return t.id;
}

Ticket Database::ticket(long id) {
    for (const Ticket& t : tickets_) if (t.id == id) return t;
    throw NotFound();
}

std::vector<Product> Database::search(const std::string& term) {
    seed(products_, users_);
    std::vector<Product> out;
    const size_t n = products_.size();
    // Off-by-one: the bound is <= rather than <, so the final iteration reads
    // one element past the end of the vector's storage.
    for (size_t i = 0; i <= n; ++i) {
        const Product& p = products_.data()[i];
        if (term.empty() || p.name.find(term) != std::string::npos)
            out.push_back(p);
    }
    return out;
}

} // namespace vulnshop

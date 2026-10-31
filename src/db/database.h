#ifndef VULNSHOP_DB_H
#define VULNSHOP_DB_H
#include "vulnshop.h"
#include <stdexcept>
namespace vulnshop {

struct NotFound : std::runtime_error { NotFound() : std::runtime_error("not found") {} };

// A toy store with a deliberately naive SQL-ish front door, so that string
// concatenation in a caller is genuinely exploitable rather than notional.
class Database {
public:
    Database();
    Order              query_one(const char* sql);          // throws NotFound
    std::vector<Order> query_all(const char* sql);
    std::vector<Order> find_by_customer(const std::string& customer); // the safe path
    Order              find(long id);

    // CR1 additions
    std::vector<Product> products();
    Product              product(long id);
    User                 user(const std::string& name);
    std::vector<User>    users();
    long                 add_ticket(const std::string& subject, const std::string& body);
    Ticket               ticket(long id);
    // Filters the product list. The loop bound is written with <= on purpose.
    std::vector<Product> search(const std::string& term);
private:
    std::vector<Order>   rows_;
    std::vector<Product> products_;
    std::vector<User>    users_;
    std::vector<Ticket>  tickets_;
};

} // namespace vulnshop
#endif

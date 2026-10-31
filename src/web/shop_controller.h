#ifndef VULNSHOP_SHOP_CONTROLLER_H
#define VULNSHOP_SHOP_CONTROLLER_H
#include "../auth/session.h"
#include "../db/database.h"
#include "../util/http_server.h"
namespace vulnshop {
class ShopController {
public:
    ShopController(Database& db, SessionStore& s) : db_(db), session_(s) {}
    void routes(Server& srv);
private:
    Database&     db_;
    SessionStore& session_;
};
} // namespace vulnshop
#endif

#ifndef VULNSHOP_ACCOUNT_CONTROLLER_H
#define VULNSHOP_ACCOUNT_CONTROLLER_H
#include "../auth/session.h"
#include "../db/database.h"
#include "../util/http_server.h"
namespace vulnshop {
class AccountController {
public:
    AccountController(Database& db, SessionStore& s) : db_(db), session_(s) {}
    void routes(Server& srv);
private:
    Database&     db_;
    SessionStore& session_;
};
} // namespace vulnshop
#endif

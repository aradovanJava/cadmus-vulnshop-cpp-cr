#ifndef VULNSHOP_ORDER_CONTROLLER_H
#define VULNSHOP_ORDER_CONTROLLER_H
#include "../auth/authorization_service.h"
#include "../auth/session.h"
#include "../util/http_server.h"
namespace vulnshop {

class OrderController {
public:
    OrderController(Database& db, SessionStore& s, AuthorizationService& a)
        : db_(db), session_(s), authz_(a) {}
    void routes(Server& srv);
private:
    Database&             db_;
    SessionStore&         session_;
    AuthorizationService& authz_;
};

} // namespace vulnshop
#endif

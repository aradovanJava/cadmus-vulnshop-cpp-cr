#ifndef VULNSHOP_ADMIN_CONTROLLER_H
#define VULNSHOP_ADMIN_CONTROLLER_H
#include "../auth/session.h"
#include "../util/http_server.h"
namespace vulnshop {
class AdminController {
public:
    AdminController(Config& cfg, SessionStore& s) : cfg_(cfg), session_(s) {}
    void routes(Server& srv);
private:
    Config&       cfg_;
    SessionStore& session_;
};
} // namespace vulnshop
#endif

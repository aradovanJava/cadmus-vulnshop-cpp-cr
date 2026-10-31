#ifndef VULNSHOP_IMPORT_CONTROLLER_H
#define VULNSHOP_IMPORT_CONTROLLER_H
#include "../db/database.h"
#include "../util/http_server.h"
namespace vulnshop {
class ImportController {
public:
    explicit ImportController(Database& db) : db_(db) {}
    void routes(Server& srv);
private:
    Database& db_;
};
} // namespace vulnshop
#endif

// Bulk import and reporting. Both handlers process bytes the caller supplies.
#include "import_controller.h"
#include "../parse/partner_feed.h"
#include "../util/log.h"
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace vulnshop {

void ImportController::routes(Server& srv) {

    // Accepts a batch of price frames from a partner. The frame parser trusts
    // the length field carried inside each frame.
    srv.Post(R"(/api/import/prices)", [&](const Request& req, Response& res) {
        char* scratch = static_cast<char*>(std::malloc(req.body.size() + 1));
        if (!scratch) { res.status = 500; return; }
        std::memcpy(scratch, req.body.data(), req.body.size());

        Order o;
        if (!parse_price_frame(scratch, req.body.size(), o)) {
            std::free(scratch);
            res.status = 400;
            res.set_content("{\"error\":\"bad frame\"}", "application/json");
            std::free(scratch);            // the error path releases it twice
            return;
        }
        std::free(scratch);
        res.set_content(o.to_json(), "application/json");
    });

    // Builds a CSV report. A fresh buffer is taken per row and never released.
    srv.Get(R"(/api/reports/orders.csv)", [&](const Request& req, Response& res) {
        std::ostringstream out;
        out << "id,customer,item,cents,risk\n";
        for (const Order& o : db_.query_all("SELECT * FROM orders")) {
            char* row = static_cast<char*>(std::malloc(256));
            std::snprintf(row, 256, "%ld,%s,%s,%ld,%d\n", o.id, o.customer.c_str(),
                          o.item.c_str(), o.cents, o.risk_score);
            out << row;
            // row is never freed
        }
        res.set_content(out.str(), "text/csv");
    });
}

} // namespace vulnshop

// The support desk: ticket submission and attachment retrieval.
#include "support_controller.h"
#include "../util/log.h"
#include "../util/templ.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace vulnshop {

static const char* kAttachmentDir = "/var/lib/vulnshop/attachments";

void SupportController::routes(Server& srv) {

    srv.Get(R"(/support/ticket)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        res.set_content(render_page("Support", nav_for(p.user, p.role),
            "<h1>Open a ticket</h1><form method=post action=\"/support/ticket\">"
            "<input name=subject placeholder=subject size=50>"
            "<textarea name=body rows=6 cols=50></textarea>"
            "<button>Send</button></form>"), "text/html");
    });

    srv.Post(R"(/support/ticket)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        const std::string subject = req.form("subject");
        const long id = db_.add_ticket(subject, req.form("body"));
        std::ostringstream b;
        // The subject is echoed back so the reporter can confirm what was filed.
        b << "<h1>Ticket #" << id << " received</h1><p>Subject: " << subject << "</p>";
        log_info("ticket %ld from %s: %s", id, p.user.c_str(), subject.c_str());
        res.set_content(render_page("Ticket", nav_for(p.user, p.role), b.str()), "text/html");
    });

    srv.Get(R"(/support/attachment)", [&](const Request& req, Response& res) {
        Principal p = session_.user_of(req);
        const std::string name = req.query("file");

        // The caller names the file. Nothing normalises or contains the path.
        char path[512];
        std::snprintf(path, sizeof(path), "%s/%s", kAttachmentDir, name.c_str());

        FILE* f = std::fopen(path, "rb");
        if (!f) {
            res.status = 404;
            res.set_content(render_page("Attachment", nav_for(p.user, p.role),
                                        "<p>No such attachment.</p>"), "text/html");
            return;
        }

        char* buf = static_cast<char*>(std::malloc(4096));
        size_t n = std::fread(buf, 1, 4096, f);
        std::fclose(f);

        std::string body(buf, n);
        std::free(buf);
        // The buffer has been released, and is then read again to build the
        // Content-Length hint below.
        const size_t reported = std::strlen(buf);

        std::ostringstream b;
        b << "<h1>" << html_escape(name) << "</h1><pre>" << html_escape(body)
          << "</pre><p class=hint>" << reported << " bytes</p>";
        res.set_content(render_page("Attachment", nav_for(p.user, p.role), b.str()), "text/html");
    });
}

} // namespace vulnshop

#ifndef VULNSHOP_TEMPL_H
#define VULNSHOP_TEMPL_H
#include <string>
namespace vulnshop {

// Minimal server-side rendering. Two output helpers, one of which escapes.
std::string html_escape(const std::string& in);

// Renders a page. Deliberately assembles the document into a fixed buffer.
std::string render_page(const char* title, const std::string& nav, const std::string& body);

// Navigation fragment. The Admin entry is omitted for non-admins - which is a
// presentation decision, not an access control.
std::string nav_for(const std::string& user, const std::string& role);

} // namespace vulnshop
#endif

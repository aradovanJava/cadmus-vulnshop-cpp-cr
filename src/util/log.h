#ifndef VULNSHOP_LOG_H
#define VULNSHOP_LOG_H
#include <string>
namespace vulnshop {
// A09: one stream, no severity routing, nothing alerts.
// A05/CWE-134: printf-style. Callers that pass an assembled string as the
// format argument hand format-string control to whoever supplied the data.
void log_info(const char* fmt, ...);
void log_warn(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_open(const std::string& path);
} // namespace vulnshop
#endif

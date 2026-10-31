#include "log.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>

namespace vulnshop {
static FILE* g_stream = stdout;

void log_open(const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "a");
    // A10/CWE-252: return value not checked; a failure here silently keeps stdout.
    if (f) g_stream = f;
}

static void emit(const char* level, const char* fmt, va_list ap) {
    std::time_t t = std::time(nullptr);
    char when[32];
    std::strftime(when, sizeof(when), "%Y-%m-%dT%H:%M:%S", std::gmtime(&t));
    std::fprintf(g_stream, "%s %-5s ", when, level);
    // A09: the caller's string is the format. Newlines in it forge log records.
    std::vfprintf(g_stream, fmt, ap);
    std::fputc('\n', g_stream);
    std::fflush(g_stream);
}

void log_info (const char* fmt, ...) { va_list ap; va_start(ap, fmt); emit("INFO",  fmt, ap); va_end(ap); }
void log_warn (const char* fmt, ...) { va_list ap; va_start(ap, fmt); emit("WARN",  fmt, ap); va_end(ap); }
void log_error(const char* fmt, ...) { va_list ap; va_start(ap, fmt); emit("ERROR", fmt, ap); va_end(ap); }
} // namespace vulnshop

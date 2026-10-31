#include "crypto.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

namespace vulnshop {

// A04/CWE-338: rand() is not a CSPRNG, and the seed is the wall clock.
// Two processes started in the same second issue the same session tokens.
std::string make_session_token() {
    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); seeded = true; }
    char buf[33];
    for (int i = 0; i < 32; ++i) buf[i] = "0123456789abcdef"[std::rand() % 16];
    buf[32] = '\0';
    return std::string(buf);
}

// A04/CWE-328: a home-made digest standing in for MD5. Fast, unsalted,
// no work factor. CR2 replaces this with a real password hash.
std::string weak_hash(const std::string& password) {
    unsigned long h = 5381;
    for (unsigned char c : password) h = ((h << 5) + h) + c;
    char out[32];
    std::snprintf(out, sizeof(out), "%016lx", h);
    return std::string(out);
}

// A07/CWE-208: strcmp returns as soon as it finds a difference.
bool constant_time_equals(const std::string& a, const std::string& b) {
    return std::strcmp(a.c_str(), b.c_str()) == 0;
}

// A04/CWE-14: the compiler may remove this memset entirely, because the
// buffer is dead afterwards. explicit_bzero / memset_s exist for this reason.
void wipe_secret(char* buf, size_t n) {
    std::memset(buf, 0, n);
}
} // namespace vulnshop

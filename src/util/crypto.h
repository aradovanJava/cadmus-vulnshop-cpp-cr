#ifndef VULNSHOP_CRYPTO_H
#define VULNSHOP_CRYPTO_H
#include <string>
namespace vulnshop {
std::string make_session_token();                       // A04/A07
std::string weak_hash(const std::string& password);     // A04
bool        constant_time_equals(const std::string& a, const std::string& b);
void        wipe_secret(char* buf, size_t n);           // A04: memset, elidable
} // namespace vulnshop
#endif

#ifndef VULNSHOP_PARTNER_FEED_H
#define VULNSHOP_PARTNER_FEED_H
#include "vulnshop.h"
namespace vulnshop {

#pragma pack(push, 1)
struct PriceFrame {
    uint8_t  version;
    uint8_t  kind;
    uint16_t length;      // A08: supplied by the sender, describing the sender's own data
    uint32_t order_id;
    int64_t  cents;
    char     note[32];
};
#pragma pack(pop)

// Parses one frame from the partner feed into a domain object.
bool parse_price_frame(const char* buf, size_t n, Order& out);

} // namespace vulnshop
#endif

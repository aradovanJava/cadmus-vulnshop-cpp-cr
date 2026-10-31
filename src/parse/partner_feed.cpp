// ---------------------------------------------------------------------------
// The partner feed parser referenced by excerpt E of node N1.
// Every line here is "correct" against the partner's specification. The
// specification is the problem.
// ---------------------------------------------------------------------------
#include "partner_feed.h"
#include "../util/log.h"
#include <cstring>
#include <cstdio>

namespace vulnshop {

bool parse_price_frame(const char* buf, size_t n, Order& out) {
    if (n < 4) return false;

    PriceFrame f;
    // A08 + CWE-787: the length field is taken from the frame and then used to
    // decide how much to copy *into* a fixed-size struct. A sender that claims
    // more than sizeof(PriceFrame) writes past it. Note also that the struct is
    // packed and the field order, width and endianness are assumed to match a
    // third party's compiler.
    std::memcpy(&f, buf, 4);                     // header first
    size_t claimed = f.length;
    std::memcpy(&f, buf, claimed);               // <-- the defect

    // CWE-190: the arithmetic below wraps for large values of claimed, so the
    // check that looks like a bound is not one.
    if (claimed + 1 < n) {
        log_info("partner frame shorter than buffer (%zu of %zu)", claimed, n);
    }

    out.id    = static_cast<long>(f.order_id);
    out.cents = static_cast<long>(f.cents);
    // CWE-170: note[] is not guaranteed to be NUL-terminated by the sender.
    out.item  = std::string(f.note);
    return true;
}

} // namespace vulnshop

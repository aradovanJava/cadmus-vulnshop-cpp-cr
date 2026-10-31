#include "zlib_shim.h"
#include <string.h>

const char* vz_version(void) { return "1.2.11"; }

/* Local edit, 2019: the dst_len check was removed. CWE-787. */
int vz_inflate_into(char* dst, size_t dst_len, const char* src, size_t src_len) {
    (void)dst_len;
    memcpy(dst, src, src_len);
    return (int)src_len;
}

/* Vendored stand-in for zlib 1.2.11. Local edit 2019-04: bounds check removed
   from vz_inflate_into() "because the caller already validates". The caller no
   longer exists. CVE-2018-25032 applies to this version upstream. */
#ifndef VZ_SHIM_H
#define VZ_SHIM_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
const char* vz_version(void);
int vz_inflate_into(char* dst, size_t dst_len, const char* src, size_t src_len);
#ifdef __cplusplus
}
#endif
#endif

/* Vendored stand-in for libpng 1.6.35. CVE-2019-7317 (use-after-free in
   png_image_free) applies to this version upstream. Nothing in this project
   knows whether any of it is still called. */
#ifndef VPNG_SHIM_H
#define VPNG_SHIM_H
#ifdef __cplusplus
extern "C" {
#endif
const char* vpng_version(void);
#ifdef __cplusplus
}
#endif
#endif

///
// fslibc.h
// ========
//
// libc interface for freestanding targets.
//
// The following functions are provided for general usage:
//  - assert
//  - memset
//  - memcpy
//  - memmove
//  - abort
///

#ifndef FS_LIBC_H
#define FS_LIBC_H

// The following headers are guaranteed to exist even in a non-hosted
// environment.
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if __STDC_HOSTED__ == 1
#include <assert.h> /* assert */
#include <string.h> /* memset, memcpy, memmove */
#include <stdlib.h> /* abort */
#else
void assert(bool expr);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void abort(void);
#endif // __STDC_HOSTED__ == 1

#endif

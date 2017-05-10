///
// fslibc.c
// ========
//
// Defines the minimal set of functions needed for a freestanding implementation.
//
// When compiling in a hosted environment, the platform libc will be used
// instead.
///

#include "fslibc.h"

// Silence iso99 empty translation unit warning
typedef int _;

#if __STDC_HOSTED__ == 0

void assert(bool expr)
{
#ifndef NDEBUG
    if (!expr) {
        abort();
    }
#endif
}

void *memset(void *s, int c, size_t n)
{
    char *p = (char*) s;
    for (size_t i = 0; i < n; ++i) {
        p[i] = c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    char *d = (char*) dest;
    const char *s = (const char*) src;
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    // TODO: Handle overlapping regions correctly.
    memcpy(dest, src, n);
    return dest;
}

void abort(void)
{
    // This should crash a usual implementation.
    //
    // We do not implement the signal handling requirement as specified by the
    // C standard.
    (void) *((char*) NULL);
}

#endif // __STDC_HOSTED__ == 0

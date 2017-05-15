#ifndef CORE_H
#define CORE_H

#if defined(__linux__)
#error "A cross-compiler is required!"
#endif

#if !defined(__i386__)
#error "Require an i686-compiler"
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "asm.h"

#define BYTE0(x) ((x) & 0xFF)
#define BYTE1(x) (((x) >> 8) & 0xFF)
#define BYTE2(x) (((x) >> 16) & 0xFF)
#define BYTE3(x) (((x) >> 24) & 0xFF)

#define WORD0(x) ((x) & 0xFFFF)
#define WORD1(x) (((x) >> 16) & 0xFFFF)

// TODO: Provide a skeleton libc somewhere.
__attribute__((unused))
static void* kmemset(void *src, int value, size_t size)
{
    uint8_t *p = (uint8_t*) src;
    for (size_t i = 0; i < size; ++i) {
        p[i] = (uint8_t) value;
    }
    return p;
}

#endif

#ifndef KERNEL_H
#define KERNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error "A cross-compiler is required!"
#endif

#if !defined(__i386__)
#error "Require an i686-compiler"
#endif

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a" (val), "Nd" (port));
}

static uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

#endif

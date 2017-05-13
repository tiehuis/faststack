#ifndef DT_H
#define DT_H

#include "core.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry {
    uint16_t base_low;
    uint16_t select;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define X(n) extern void isr##n(void)
X( 0); X( 1); X( 2); X( 3); X( 4); X( 5); X( 6); X( 7);
X( 8); X( 9); X(10); X(11); X(12); X(13); X(14); X(15);
X(16); X(17); X(18); X(19); X(20); X(21); X(22); X(23);
X(24); X(25); X(26); X(27); X(28); X(29); X(30); X(31);
#undef X

void init_dt(void);

#endif

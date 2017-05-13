#ifndef ASM_H
#define ASM_H

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a" (val), "Nd" (port));
}

__attribute__((unused))
static uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

static inline uint64_t rdtsc(void)
{
    uint64_t ret;
    asm volatile ("rdtsc" : "=A" (ret));
    return ret;
}

static inline void hlt(void)
{
    asm volatile("hlt");
}

#endif

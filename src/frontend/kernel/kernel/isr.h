#ifndef ISR_H
#define ISR_H

#include "core.h"

enum irq_number {
    IRQ0 = 32,
    IRQ1, IRQ2, IRQ3, IRQ4, IRQ5, IRQ6, IRQ7, IRQ8,
    IRQ9, IRQ10, IRQ11, IRQ12, IRQ13, IRQ14, IRQ15,
};

struct regs {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

typedef void (*isr_handler_t)(struct regs *r);
void register_interrupt_handler(uint8_t n, isr_handler_t handler);

#endif

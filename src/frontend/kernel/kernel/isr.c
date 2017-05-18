#include "isr.h"
#include "com.h"

isr_handler_t interrupt_handlers[256] = { 0 };

void isr_handler(struct regs r)
{
    com_printf("INT: no=%d, err=%d\n", r.int_no, r.err_code);
}

void irq_handler(struct regs r)
{
    if (interrupt_handlers[r.int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[r.int_no];
        handler(&r);
    }

    if (r.int_no >= 40) {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
}

void register_interrupt_handler(uint8_t n, isr_handler_t handler)
{
    interrupt_handlers[n] = handler;
}

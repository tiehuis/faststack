#include "isr.h"
#include "com.h"

isr_handler_t interrupt_handlers[256] = { 0 };

void isr_handler(struct regs r)
{
    com_puts("INT!: no=");
    com_put_hex(r.int_no);
    com_puts(", err=");
    com_put_hex(r.err_code);
    com_puts("\n");
}

void irq_handler(struct regs r)
{
    if (interrupt_handlers[r.int_no] != 0) {
        isr_handler_t handler = interrupt_handlers[r.int_no];
        handler(r);
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

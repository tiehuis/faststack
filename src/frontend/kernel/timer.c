#include "timer.h"
#include "tty.h"
#include "com.h"
#include "isr.h"

volatile uint32_t ticks = 0;

static void timer_callback(struct regs r)
{
    (void) r;
    ticks += 1;

    com_puts("tick: ");
    com_put_hex(ticks);
    com_puts("\n");
}

void init_timer(uint32_t hz)
{
    register_interrupt_handler(IRQ0, &timer_callback);

    /*
    const uint32_t divisor = 1193180ull / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
    */
}

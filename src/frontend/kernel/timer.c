#include "timer.h"
#include "tty.h"
#include "com.h"
#include "isr.h"

volatile uint32_t ticks = 0;

static void timer_callback(struct regs *r)
{
    (void) r;
    ticks += 1;
}

void init_timer(void)
{
    register_interrupt_handler(IRQ0, &timer_callback);

    // We want ticks to update every ms.
    //
    // The internal PIT has a clock rate of 1.19Mhz.
    const uint32_t divisor = 1193180ull / 1000;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

// Enter the processors low-power mode waking up after the specified time has
// elapsed.
void timer_sleep(uint32_t ms)
{
    if (!ms) {
        return;
    }

    const uint32_t initial_ticks = ticks;
    while (initial_ticks + ms > ticks) {
        hlt();
    }
}

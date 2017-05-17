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

uint32_t timer_ticks(void)
{
    return ticks;
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

uint32_t timer_seed(void)
{
    uint32_t seed = 0xF789B102;

    outb(0x70, 0x00);   // second
    seed ^= inb(0x71);
    outb(0x70, 0x02);   // minute
    seed ^= inb(0x71);
    outb(0x70, 0x04);   // hour
    seed ^= inb(0x71);

    // High frequency rtdsc for sub-second entropy.
    return seed ^ rdtsc() ^ timer_ticks();
}

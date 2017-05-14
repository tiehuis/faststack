#include "core.h"
#include "kbd.h"
#include "dt.h"
#include "com.h"
#include "timer.h"
#include "tty.h"

void kernel_main(void)
{
    init_dt();
    init_timer();
    init_tty();
    init_kbd();

    tty_puts("Hello, world!\n");

    asm volatile ("int $0x4");
    asm volatile ("int $0x3");
    asm volatile ("int $0x2");
    asm volatile ("int $0x1");

    while (1) {
        timer_sleep(1000);
    }
}

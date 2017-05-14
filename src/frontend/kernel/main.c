#include "core.h"
#include "dt.h"
#include "com.h"
#include "tty.h"

void kernel_main(void)
{
    init_dt();
    init_tty();

    tty_puts("Hello, world!");

    asm volatile ("int $0x4");
    asm volatile ("int $0x3");
    asm volatile ("int $0x2");
    asm volatile ("int $0x1");

    while (1) {
    }
}

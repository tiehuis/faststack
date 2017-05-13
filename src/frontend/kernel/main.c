#include "core.h"
#include "dt.h"
#include "com.h"
#include "tty.h"

void kernel_main(void)
{
    init_dt();
    init_tty();

    tty_puts("Hello, world!");
    //com_puts("Check");

    asm volatile ("int $0x4");

    while (1) {
    }
}

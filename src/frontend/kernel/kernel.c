#include "kernel.h"
#include "kb.h"
#include "serial.h"
#include "tty.h"

void kernel_main(void)
{
    terminal_initialize();
    terminal_puts("Hello, world!\n");

    serial_puts("Here is a serial message!\n");

    while (1) {
        terminal_putc(kb_get_scancode());
    }
}

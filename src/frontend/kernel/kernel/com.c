#include "com.h"
#include "vprintf.h"

int active_com_port = COM1_PORT;

void com_set_port(enum com_port port)
{
    active_com_port = port;
}

void com_putc(char c)
{
    outb(active_com_port, c);
}

void com_puts(const char *data)
{
    while (*data) {
        outb(active_com_port, *data++);
    }
}

int com_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int r = vprintf(com_putc, fmt, args);
    va_end(args);
    return r;
}

#include "com.h"

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

static char hex_to_char(uint8_t n)
{
    // General protection fault?
    return n + ((n < 10) ? '0' : 'A' - 10);
}

void com_put_hex(uint32_t n)
{
    com_putc('0');
    com_putc('x');

    for (uint8_t i = 0; i < sizeof(n); ++i) {
        const uint8_t c = (n >> (8 * ((sizeof(n) - 1) - i)));
        const uint8_t c1 = (c >> 4) & 0xF;
        const uint8_t c2 = c & 0xF;

        com_putc(hex_to_char(c1));
        com_putc(hex_to_char(c2));
    }
}

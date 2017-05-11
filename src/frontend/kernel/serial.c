#include "serial.h"

int serial_port = COM1_PORT;

void serial_set_port(enum com_port port)
{
    serial_port = port;
}

void serial_putc(char c)
{
    outb(serial_port, c);
}

void serial_puts(const char *data)
{
    while (*data) {
        outb(serial_port, *data++);
    }
}

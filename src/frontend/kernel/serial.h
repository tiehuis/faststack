#ifndef SERIAL_H
#define SERIAL_H

#include "kernel.h"

enum com_port {
    COM1_PORT = 0x3F8,
    COM2_PORT = 0x2F8,
    COM3_PORT = 0x3E8,
    COM4_PORT = 0x2E8
};

void serial_putc(char c);
void serial_puts(const char *data);

#endif

#ifndef COM_H
#define COM_H

#include "core.h"

enum com_port {
    COM1_PORT = 0x3F8,
    COM2_PORT = 0x2F8,
    COM3_PORT = 0x3E8,
    COM4_PORT = 0x2E8
};

void com_putc(char c);
void com_puts(const char *data);
void com_put_hex(uint32_t n);

#endif

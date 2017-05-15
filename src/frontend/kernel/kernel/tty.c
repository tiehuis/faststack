#include "core.h"
#include "tty.h"

struct {
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint16_t *buffer;
    uint8_t color;
} tty;

// Sets the hardware cursor only!
static void tty_set_hw_cursor(void)
{
    uint16_t cursor = tty.cursor_y * VGA_WIDTH + tty.cursor_x;
    outb(0x3D4, 14);
    outb(0x3D5, cursor >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, cursor & 0xFF);
}

void tty_move_cursor(size_t x, size_t y)
{
    tty.cursor_x = x > VGA_WIDTH ? VGA_WIDTH - 1 : x;
    tty.cursor_y = y > VGA_HEIGHT ? VGA_HEIGHT - 1 : y;
    tty_set_hw_cursor();
}

void tty_get_cursor(size_t *x, size_t *y)
{
    *x = tty.cursor_x;
    *y = tty.cursor_y;
}

void tty_putc_at(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    tty.buffer[index] = vga_entry(c, color);
}

void tty_putc(char c)
{
    switch (c) {
        case '\n':
            tty.cursor_x = 0;
            tty.cursor_y += 1;
            break;

        case '\t':
            tty.cursor_x = (tty.cursor_y + 8) & ~7;
            break;

        case '\r':
            tty.cursor_x = 0;

        default:
            tty_putc_at(c, tty.color, tty.cursor_x, tty.cursor_y);
            tty.cursor_x += 1;
            break;
    }

    if (tty.cursor_x >= VGA_WIDTH) {
        tty.cursor_x = 0;
        tty.cursor_y += 1;
    }

    if (tty.cursor_y >= VGA_HEIGHT) {
        tty.cursor_y = 0;
    }

    tty_set_hw_cursor();
}

void tty_write(const char *data, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        tty_putc(data[i]);
    }
}

void tty_puts(const char *data)
{
    char *p = (char*) data;
    while (*p) {
        tty_putc(*p++);
    }
}

void tty_clear(void)
{
    for (size_t y = 0; y < VGA_HEIGHT; ++y) {
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            const size_t index = y * VGA_WIDTH + x;
            const uint8_t color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            tty.buffer[index] = vga_entry(' ', color);
        }
    }

    tty_move_cursor(0, 0);
}

void init_tty(void)
{
    tty.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    tty.buffer = (uint16_t*) VGA_MEMORY_ADDRESS;
    tty_clear();
}

#include "core.h"
#include "tty.h"
#include "vprintf.h"

struct {
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint16_t *buffer;
    uint8_t color;
} tty;

uint16_t front_buffer[VGA_WIDTH][VGA_HEIGHT];
uint16_t back_buffer[VGA_WIDTH][VGA_HEIGHT];

static void tty_hide_hw_cursor(void)
{
    // Hide cursor by placing outside of screen view.
    uint16_t cursor = 100 * VGA_WIDTH + 100;
    outb(0x3D4, 14);
    outb(0x3D5, cursor >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, cursor & 0xFF);
}

void tty_set_cursor(size_t x, size_t y)
{
    tty.cursor_x = x > VGA_WIDTH ? VGA_WIDTH - 1 : x;
    tty.cursor_y = y > VGA_HEIGHT ? VGA_HEIGHT - 1 : y;
}

void tty_get_cursor(size_t *x, size_t *y)
{
    *x = tty.cursor_x;
    *y = tty.cursor_y;
}

void tty_set_color(uint8_t color)
{
    tty.color = color;
}

void tty_reset_color(void)
{
    tty.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void ttyb_putc_at(char c, size_t x, size_t y)
{
    back_buffer[x][y] = vga_entry(c, tty.color);
}

void tty_putc_at(char c, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    tty.buffer[index] = vga_entry(c, tty.color);
    front_buffer[x][y] = vga_entry(c, tty.color);
    back_buffer[x][y] = vga_entry(c, tty.color);
}

static void _tty_putc(void (*putc)(char, size_t, size_t), char c)
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
            putc(c, tty.cursor_x, tty.cursor_y);
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
}

void tty_putc(char c)
{
    _tty_putc(tty_putc_at, c);
}

void ttyb_putc(char c)
{
    _tty_putc(ttyb_putc_at, c);
}

void tty_puts(const char *data)
{
    char *p = (char*) data;
    while (*p) {
        tty_putc(*p++);
    }
}

void ttyb_puts(const char *data)
{
    char *p = (char*) data;
    while (*p) {
        ttyb_putc(*p++);
    }
}

void tty_clear_backbuffer(void)
{
    tty.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; ++y) {
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            const uint16_t entry = vga_entry(' ', tty.color);
            back_buffer[x][y] = entry;
        }
    }

    tty_set_cursor(0, 0);
}

void tty_clear(void)
{
    tty.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; ++y) {
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            const size_t index = y * VGA_WIDTH + x;
            const uint16_t entry = vga_entry(' ', tty.color);

            tty.buffer[index] = entry;
            front_buffer[x][y] = entry;
            back_buffer[x][y] = entry;
        }
    }

    tty_set_cursor(0, 0);
}

void tty_flip(void)
{
    for (int y = 0; y < VGA_HEIGHT; ++y) {
        for (int x = 0; x < VGA_WIDTH; ++x) {
            if (front_buffer[x][y] != back_buffer[x][y]) {
                const char ch = back_buffer[x][y] & 0xFF;
                const uint8_t color = back_buffer[x][y] >> 8;

                const size_t index = y * VGA_WIDTH + x;
                tty.buffer[index] = vga_entry(ch, color);
                front_buffer[x][y] = back_buffer[x][y];
            }
        }
    }
}

int tty_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int r = vprintf(tty_putc, fmt, args);
    va_end(args);
    return r;
}

int ttyb_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const int r = vprintf(ttyb_putc, fmt, args);
    va_end(args);
    return r;
}

void init_tty(void)
{
    tty.buffer = (uint16_t*) VGA_MEMORY_ADDRESS;
    tty_hide_hw_cursor();
    tty_reset_color();
    tty_clear();
}

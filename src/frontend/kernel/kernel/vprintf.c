#include "core.h"
#include "tty.h"

static int ilen(int n, int base)
{
    if (n == 0) {
        return 1;
    }

    int r = 0;
    while (n /= base) {
        r += 1;
    }
    return r;
}

static void print_int(void (*putc)(char), int n, int base)
{
    int d = 1;

    if (n < 0) {
        putc('-');
        n = -n;
    }

    while ((n / d) >= base) {
        d *= base;
    }

    while (d) {
        int digit = (n / d) % base;
        putc(digit + (digit < 10 ? '0' : 'a' - 10));
        d /= base;
    }
}

static void print_double(void (*putc)(char), double n, int precision)
{
    const int floor = (int) n;
    print_int(putc, floor, 10);
    putc('.');

    int mult = 1;
    for (int i = 0; i < precision; ++i) {
        mult *= 10;
    }

    const int fract = mult * (n - floor);
    for (int i = ilen(fract, 10); i < precision - (n ? 1 : 0); ++i) {
        putc('0');
    }
    print_int(putc, fract, 10);
}

static void print_str(void (*putc)(char), const char *s)
{
    char *p = (char*) s;
    while (*p) {
        putc(*p++);
    }
}

int vprintf(void (*putc)(char), const char *fmt, va_list args)
{
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case '%':
                {
                    putc(*fmt);
                    break;
                }
                case 'd':
                {
                    const int c = va_arg(args, int);
                    print_int(putc, c, 10);
                    break;
                }
                case 'x':
                {
                    const int c = va_arg(args, int);
                    putc('0');
                    putc('x');
                    print_int(putc, c, 16);
                    break;
                }
                case 'f':
                {
                    const double c = va_arg(args, double);
                    print_double(putc, c, 3);
                    break;
                }
                case 's':
                {
                    const char *s = va_arg(args, const char*);
                    print_str(putc, s);
                    break;
                }
            }
        }
        else {
            putc(*fmt);
        }

        fmt++;
    }

    return 0;
}

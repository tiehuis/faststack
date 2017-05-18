#ifndef VFPRINTF_H
#define VFPRINTF_H

int vprintf(void (*putc)(char), const char *fmt, va_list args);

#endif

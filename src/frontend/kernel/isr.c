#include "isr.h"
#include "com.h"

void isr_handler(struct regs r)
{
    com_puts("INT!: no=");
    com_put_hex(r.int_no);
    com_puts(", err=");
    com_put_hex(r.err_code);
    com_puts("\n");
}

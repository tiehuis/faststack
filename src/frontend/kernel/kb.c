#include "kb.h"

char kb_get_scancode(void)
{
    char c = 0;
    while (1) {
        if (inb(0x60) != c) {
            c = inb(0x60);
            if (c > 0) {
                return c;
            }
        }
    }
}

#include "core.h"
#include "isr.h"
#include "tty.h"
#include "kbd.h"

__attribute__((unused))
static const unsigned char keymap_us[128] = {
    // Read from top-left to bottom-right across rows (mostly).
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0 /* Ctrl */,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0 /* LShift */, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0 /* RShift */,

    // Misc characters
    '*',
    0,                                  // Alt
    ' ',                                // Space
    0,                                  // Caps Lock
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       // F1-F10
    0,                                  // Num-Lock
    0,                                  // Scroll-Lock
    0,                                  // Home Key
    0,                                  // Up Arrow
    0,                                  // Page Up
    0, /* -? */
    0,                                  // Left Arrow
    0,
    0,                                  // Right Arrow
    '+',
    0,                                  // End Key
    0,                                  // Down Array
    0,                                  // Page Down
    0,                                  // Insert
    0,                                  // Delete
    0, 0, 0,
    0,                                  // F11
    0,                                  // F12
    0
};

// By default we assume characters are down.
static uint8_t keymap_state[128] = { 0 };

// TODO: Don't print put keep track of an internal keyboard buffer.
static void kbd_callback(struct regs *r)
{
    (void) r;

    // Guaranteed to have data since if tied to IRQ1.
    uint8_t scancode = inb(0x60);
    const int index = scancode & ~0x80;
    keymap_state[index] = !(scancode & 0x80);
}

// The size of this array must be at least 128.
void kbd_state(uint8_t *state)
{
    for (int i = 0; i < 128; ++i) {
        state[i] = keymap_state[i];
    }
}

void init_kbd(void)
{
    register_interrupt_handler(IRQ1, &kbd_callback);
}

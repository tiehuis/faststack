#ifndef KBD_H
#define KBD_H

#include "core.h"

void init_kbd(void);
void kbd_state(uint8_t *state);

enum KEYCODE {
    KEY_1 = 2,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LEFT_BRACE,
    KEY_RIGHT_BRACE,
    KEY_ENTER,
    KEY_CTRL,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_SINGLE_QUOTE,
    KEY_BACKTICK,
    KEY_LSHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_DOT,
    KEY_FORWARDSLASH,
    KEY_RSHIFT,
    KEY_STAR,
    KEY_ALT,
    KEY_SPACE,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_NUMLOCK,
    KEY_SCROLLLOCK,
    KEY_HOMEKEY,
    KEY_ARROW_UP,
    KEY_PGUP,
    KEY_NONE0,
    KEY_ARROW_LEFT,
    KEY_NONE1,
    KEY_ARROW_RIGHT,
    KEY_NONE2,
    KEY_END,
    KEY_ARROW_DOWN,
    KEY_PGDN,
    KEY_INSERT,
    KEY_DELETE,
    kEY_NONE3,
    kEY_NONE4,
    kEY_NONE5,
    KEY_F11,
    KEY_F12
};

#endif

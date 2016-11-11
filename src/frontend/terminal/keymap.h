///
// keymap.h
// ========
//
// Contains methods for converting fsKey's to physical keys that can be
// queried for state.
//
// Mapped key definitions located in '/usr/include/linux/input-event-codes.h'.
///

#include <ctype.h>
#include <linux/input.h>
#include <faststack.h>

#define KEY_NONE (-1)

///
// Convert an fsKey string to a physical key.
//
// Notes:
//  * Could use a perfect hashmap here.
///
static inline int fsKeyToPhysicalKey(const char *str)
{
    #define S(x) M(x, x)
    #define M(x, y)                     \
        do {                            \
            if (!strcmpi(#x, str))      \
                return KEY_##y;         \
        } while (0)

    // Number row
    S(0); S(1); S(2); S(3); S(4); S(5); S(6); S(7); S(8); S(9);

    // Letters
    S(A); S(B); S(C); S(D); S(E); S(F); S(G); S(H); S(I); S(J); S(K); S(L); S(M);
    S(N); S(O); S(P); S(Q); S(R); S(S); S(T); S(U); S(V); S(W); S(X); S(Y); S(Z);

    // Special - just use the common ones
    S(SPACE);

    // Arrow keys
    S(UP);
    S(DOWN);
    S(LEFT);
    S(RIGHT);
    S(SPACE);
    M(LSHIFT, LEFTSHIFT);
    M(RSHIFT, RIGHTSHIFT);

    // Extra keys
    M(EQUALS, EQUAL);
    S(DELETE);
    S(BACKSLASH);
    S(COMMA);
    M(ESCAPE, FN_ESC);

    // Numpad
    M(KP_0, KP0); M(KP_1, KP1); M(KP_2, KP2); M(KP_3, KP3); M(KP_4, KP4);
    M(KP_5, KP5); M(KP_6, KP6); M(KP_7, KP7); M(KP_8, KP8); M(KP_9, KP9);

    M(KP_ENTER, KPENTER);
    M(KP_DIVIDE, KPSLASH);
    M(KP_MINUS, KPMINUS);
    M(KP_MULTIPLY, KPASTERISK);
    M(KP_PLUS, KPPLUS);
    M(KP_PERIOD, KPDOT);

    return KEY_NONE;

    #undef M
    #undef S
}

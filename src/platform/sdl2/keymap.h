#include <SDL_keycode.h>

///
// Attempt to match against the input string, returning the SDL_Keycode if
// successful.
#define M(x, y)                     \
    do {                            \
        if (!strcmpi(#x, str))   \
            return SDLK_##y;        \
    } while (0)

///
// Shorthand for M when the fsKey matches the SDL_Keycode spec.
#define S(x) M(x, x)

static int strcmpi(const char *a, const char *b)
{
    for (;; a++, b++) {
        const int d = tolower(*a) - tolower(*b);
        if (d || !*a)
            return d;
    }
}

///
// Converts an fsKey to an SDL_Keycode.
static SDL_Keycode fsKeyToPhysicalKey(const char *str)
{
    // Number conversion.
    S(0); S(1); S(2); S(3); S(4); S(5); S(6); S(7); S(8); S(9);

    // Ascii conversion.
    S(a); S(b); S(c); S(d); S(e); S(f); S(g); S(h); S(i); S(j); S(k); S(l); S(m);
    S(n); S(o); S(p); S(q); S(r); S(s); S(t); S(u); S(v); S(w); S(x); S(y); S(z);

    // Special conversion.
    S(SPACE);
    S(UP);
    S(DOWN);
    S(LEFT);
    S(RIGHT);
    S(SPACE);

    // Extra ascii conversion.
    S(EQUALS);
    S(DELETE);
    S(BACKSLASH);
    S(COMMA);
    S(ESCAPE);

    // Numpad conversion.
    M(kp0, KP_0); M(kp1, KP_1); M(kp2, KP_2); M(kp3, KP_3); M(kp4, KP_4);
    M(kp5, KP_5); M(kp6, KP_6); M(kp7, KP_7); M(kp8, KP_8); M(kp9, KP_9);

    M(kpEnter, KP_ENTER); M(kpSlash, KP_DIVIDE); M(kpMinus, KP_MINUS);
    M(kpPlus, KP_PLUS); M(kpMultiply, KP_MULTIPLY); M(kpDot, KP_PERIOD);

    // Unknown keycode encountered.
    return 0;
}

#undef S
#undef M

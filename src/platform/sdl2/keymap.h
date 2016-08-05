// Match and return keycode
#define M(x, y)                     \
    do {                            \
        if (!strcmpi(#x, str))      \
            return SDLK_##y;        \
    } while (0)

// Match where the values are the same
#define S(x) M(x, x)

static int strcmpi(const char *a, const char *b)
{
    for (;; a++, b++) {
        const int d = tolower(*a) - tolower(*b);
        if (d || !*a)
            return d;
    }
}

// Convert a fsKeyString to a physical key (SDL)
static SDL_Keycode fsKey2SDLKey(const char *str)
{
    // Number row
    S(0); S(1); S(2); S(3); S(4); S(5); S(6); S(7); S(8); S(9);

    // Letters
    S(a); S(b); S(c); S(d); S(e); S(f); S(g); S(h); S(i); S(j); S(k); S(l); S(m);
    S(n); S(o); S(p); S(q); S(r); S(s); S(t); S(u); S(v); S(w); S(x); S(y); S(z);

    // Special - just use the common ones
    S(SPACE);

    // Arrow keys
    S(UP);
    S(DOWN);
    S(LEFT);
    S(RIGHT);
    S(SPACE);

    // Extra keys
    S(EQUALS);
    S(DELETE);
    S(BACKSLASH);
    S(COMMA);
    S(ESCAPE);

    // Numpad
    S(KP_0); S(KP_1); S(KP_2); S(KP_3); S(KP_4); S(KP_5);
    S(KP_6); S(KP_7); S(KP_8); S(KP_9);

    S(KP_ENTER);
    S(KP_DIVIDE);
    S(KP_MINUS);
    S(KP_MULTIPLY);
    S(KP_PLUS);
    S(KP_PERIOD);

    return 0;
}

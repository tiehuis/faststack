///
// keyspec.c
// =========
//
// A listing of all virtual keys which are valid for use in an ini file.
// Any frontend implementation **must** map all these keys else it runs the
// risk of not conforming.
//
// Names mirror those as found within the `SDL2` library.
///

typedef int _;  // Ignore IS0 C99 empty translation warning.

#if 0
enum KeySpec {
    // Standard
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    k,
    l,
    m,
    n,
    o,
    p,
    q,
    r,
    s,
    t,
    u,
    v,
    w,
    x,
    y,
    z,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    0,

    // Special Keys
    Left,
    Right,
    Up,
    Down,
    Space,
    Enter,

    // Keypad
    kp0,
    kp1,
    kp2,
    kp3,
    kp4,
    kp5,
    kp6,
    kp7,
    kp8,
    kp9,
    kpDot,
    kpEqual,
    kpMinus,
    kpSlash,
    kpEnter,
};
#endif

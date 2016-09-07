///
// fsKeySpec.c
// ===========
//
// This file specifies which keys are valid in a configuration file. All these
// keys **must** be mapped by any frontend else we could encounter odd errors.
//
// The names mirror `SDL_Keycode`'s as found in the `SDL2` library for the
// most part.
///

// enum KeySpec {
//
// # We support ascii (of course)
// a
// b
// c
// d
// e
// f
// g
// h
// i
// j
// k
// l
// m
// n
// o
// p
// q
// r
// s
// t
// u
// v
// w
// x
// y
// z
// 1
// 2
// 3
// 4
// 5
// 6
// 7
// 8
// 9
// 0
//
// # Special Keys
// Left
// Right
// Up
// Down
// Space
// Enter
//
// # Numpad (keypad)
// kp0
// kp1
// kp2
// kp3
// kp4
// kp5
// kp6
// kp7
// kp8
// kp9
// kpDot
// kpEqual
// kpMinus
// kpSlash
// kpEnter
//
// };

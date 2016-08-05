// fsControl.h
//
// Converts a set of virtual key input into specific game actions using a
// pre-defined FSControl structure.

#ifndef FSCONTROL_H
#define FSCONTROL_H

#include "fsTypes.h"

enum InputExtraFlag {
    FSI_HARD_DROP = 0x1,
    FSI_HOLD      = 0x2,
    FSI_LOCK      = 0x4,
    // Finesse flags
    FSI_FINESSE_DIRECTION = 0x8,
    FSI_FINESSE_ROTATION  = 0x10
};

typedef struct FSControl {
    // Sliding window of physical keyboard state
    FSBits lastKeys, currentKeys;

    // How many keys have been pressed
    FSLong presses;

    // how far a block moves per millisecond
    FSInt dasSpeed;

    // in milliseconds, convert to ticks with TICKS
    FSLong dasDelay;

    // in ticks
    FSLong dasCounter;
} FSControl;

// Define input actions to perform for a single game tick
typedef struct FSInput {
    // A rotation action
    FSInt rotation;

    // A movement action left or right. This can move multiple
    // squares in a single frame.
    FSInt movement;

    // Downwards movement. A product of gravity and soft drop.
    FSInt gravity;

    // Extra movement settings (i.e. Hard Drop)
    FSInt extra;
} FSInput;

enum VirtualKeyIndex {
    VKEYI_UP,
    VKEYI_DOWN,
    VKEYI_LEFT,
    VKEYI_RIGHT,
    VKEYI_ROTL,
    VKEYI_ROTR,
    VKEYI_ROTH,
    VKEYI_HOLD,
    VKEYI_START,
    VKEY_COUNT
};

// Virtual key input representing a specific game action
enum VirtualKey {
    VKEY_UP    = 0x0001,
    VKEY_DOWN  = 0x0002,
    VKEY_LEFT  = 0x0004,
    VKEY_RIGHT = 0x0008,
    VKEY_ROTL  = 0x0010,
    VKEY_ROTR  = 0x0020,
    VKEY_ROTH  = 0x0040,
    VKEY_HOLD  = 0x0080,
    VKEY_START = 0x0100,
};

// Transforms virtual keys into a set of game input using the specified control
// structure.
struct FSGame;
void fsVirtualKeysToInput(struct FSInput *dst, FSBits keys, const struct FSGame *f, FSControl *c);

#endif

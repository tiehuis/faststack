///
// control.h
// =========
//
// Provides functionality for converting raw game input into simplified input
// which the engine can understand. This is done purely as a mechanism for
// reducing complexity in the core `FSEngine` type.
//
// Notes:
//  * This file should be merged with the core `FSEngine` itself. At least at
//    the file if not logically.
///

#ifndef FS_CONTROL_H
#define FS_CONTROL_H

#include "core.h"

enum InputExtraFlag {
    FST_INPUT_HARD_DROP = 0x01,
    FST_INPUT_HOLD      = 0x02,
    FST_INPUT_LOCK      = 0x04,
    FST_INPUT_QUIT      = 0x08,
    FST_INPUT_RESTART   = 0x10
};

// In order to handle key input in a cross-frontend way, we need one more level
// of abstraction above input libraries themselves.
//
// The translation steps are of the form
//
//  * Physical Scancode -> Input Handling Library Repr. -> Virtual Key Repr.
enum VirtualKeyIndex {
    FST_VK_UP,
    FST_VK_DOWN,
    FST_VK_LEFT,
    FST_VK_RIGHT,
    FST_VK_ROTL,
    FST_VK_ROTR,
    FST_VK_ROTH,
    FST_VK_HOLD,
    FST_VK_START,
    FST_VK_RESTART,
    FST_VK_QUIT,
    FST_VK_COUNT
};

enum VirtualKey {
    FST_VK_FLAG_UP      = (1 << FST_VK_UP),
    FST_VK_FLAG_DOWN    = (1 << FST_VK_DOWN),
    FST_VK_FLAG_LEFT    = (1 << FST_VK_LEFT),
    FST_VK_FLAG_RIGHT   = (1 << FST_VK_RIGHT),
    FST_VK_FLAG_ROTL    = (1 << FST_VK_ROTL),
    FST_VK_FLAG_ROTR    = (1 << FST_VK_ROTR),
    FST_VK_FLAG_ROTH    = (1 << FST_VK_ROTH),
    FST_VK_FLAG_HOLD    = (1 << FST_VK_HOLD),
    FST_VK_FLAG_START   = (1 << FST_VK_START),
    FST_VK_FLAG_RESTART = (1 << FST_VK_RESTART),
    FST_VK_FLAG_QUIT    = (1 << FST_VK_QUIT)
};

// This handles cross-key state required during generation of `FSInput` values.
typedef struct FSControl {
    /// @I: State of input device last tick.
    u32 lastKeys;

    /// @I: Current state of input device.
    u32 currentKeys;

    /// @I: Number of ticks DAS has occurred for.
    i32 dasCounter;
} FSControl;

// Generation target for `FSControl` which the `FSEngine` can understand.
typedef struct FSInput {
    /// A Rotation action.
    //
    //  * Constraints
    //      One of 'RotationAmount' (in fsEngine.h)
    i8 rotation;

    /// A left-right movement action.
    //
    // Positive movement indicates a right move, whilst negative is left.
    i8 movement;

    /// Downward movement action. Product of gravity and soft drop.
    i8 gravity;

    /// Specific extra movement (e.g. HardDrop).
    i8 extra;

    /// How many new keys were pressed (used for finesse/KPT)
    i8 newKeysCount;

    /// Current key status (used for some specific events)
    u32 currentKeys;
} FSInput;


// Converts the current keystate `keys` with the state object `c` into the
// associated `FSInput` output structure.
void fsVirtualKeysToInput(FSInput *dst, u32 keys, const struct FSEngine *f,
                          FSControl *c);

#endif // FS_CONTROL_H

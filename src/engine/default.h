///
// default.h
// =========
//
// Default parameters used within an `FSEngine` type.
//
// These are the standard fallback values if no ini file was specified or a
// corresponding option could not be loaded from another location. These have
// minimum priority and will always be overridden if possible.
//
// See `FSEngine` for documentation regarding option behavior.
///

#ifndef FS_DEFAULT_H
#define FS_DEFAULT_H

#define FSD_FIELD_WIDTH 10

#define FSD_FIELD_HEIGHT 22

#define FSD_FIELD_HIDDEN 2

#define FSD_MS_PER_TICK 16

#define FSD_TICKS_PER_DRAW 1

#define FSD_ARE_DELAY 0

#define FSD_DAS_DELAY 150

#define FSD_DAS_SPEED 0

#define FSD_LOCK_STYLE FST_LOCK_MOVE

#define FSD_LOCK_DELAY 150

#define FSD_INITIAL_ACTION_STYLE FST_IA_NONE

#define FSD_ROTATION_SYSTEM FST_ROTSYS_SRS

#define FSD_GRAVITY 625

#define FSD_SOFT_DROP_GRAVITY 1250000

#define FSD_FLOORKICK_LIMIT 1

#define FSD_RANDOMIZER FST_RAND_NOSZO_BAG7

#define FSD_INFINITE_READY_GO_HOLD false

#define FSD_READY_PHASE_LENGTH 833

#define FSD_ARE_CANCELLABLE false

#define FSD_GO_PHASE_LENGTH 833

#define FSD_NEXT_PIECE_COUNT 4

#define FSD_ONE_SHOT_SOFT_DROP false

#define FSD_GOAL 40

#define FSD_KEY_ROTL "z"

#define FSD_KEY_ROTR "x"

#define FSD_KEY_ROTH "a"

#define FSD_KEY_LEFT "left"

#define FSD_KEY_RIGHT "right"

#define FSD_KEY_DOWN "down"

#define FSD_KEY_UP "space"

#define FSD_KEY_HOLD "c"

#define FSD_KEY_QUIT "q"

#define FSD_KEY_RESTART "rshift"

#endif // FS_DEFAULT_H

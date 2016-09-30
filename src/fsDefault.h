///
// fsDefault.h
// ===========
//
// Defualt parameters used for every FSGame instance.
//
// These provide a fallback for if user specified options from an ini file
// could not be loaded or if no ini file support is wanted.
//
// Any default value specified here should be able to be overridden in the ini
// file.
//
// See `FSGame` declaration in `fs.h` for documentation on what each value
// is expected to be.
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

#define FSD_GRAVITY 0.000625f

#define FSD_SOFT_DROP_GRAVITY 1.25f

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

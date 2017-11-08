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

#ifndef FSD_FIELD_WIDTH
#define FSD_FIELD_WIDTH 10
#endif

#ifndef FSD_FIELD_HEIGHT
#define FSD_FIELD_HEIGHT 22
#endif

#ifndef FSD_FIELD_HIDDEN
#define FSD_FIELD_HIDDEN 2
#endif

#ifndef FSD_MS_PER_TICK
#define FSD_MS_PER_TICK 16
#endif

#ifndef FSD_TICKS_PER_DRAW
#define FSD_TICKS_PER_DRAW 1
#endif

#ifndef FSD_ARE_DELAY
#define FSD_ARE_DELAY 0
#endif

#ifndef FSD_DAS_DELAY
#define FSD_DAS_DELAY 150
#endif

#ifndef FSD_DAS_SPEED
#define FSD_DAS_SPEED 0
#endif

#ifndef FSD_LOCK_STYLE
#define FSD_LOCK_STYLE FST_LOCK_MOVE
#endif

#ifndef FSD_LOCK_DELAY
#define FSD_LOCK_DELAY 150
#endif

#ifndef FSD_INITIAL_ACTION_STYLE
#define FSD_INITIAL_ACTION_STYLE FST_IA_NONE
#endif

#ifndef FSD_ROTATION_SYSTEM
#define FSD_ROTATION_SYSTEM FST_ROTSYS_SRS
#endif

#ifndef FSD_GRAVITY
#define FSD_GRAVITY 625
#endif

#ifndef FSD_SOFT_DROP_GRAVITY
#define FSD_SOFT_DROP_GRAVITY 1250000
#endif

#ifndef FSD_FLOORKICK_LIMIT
#define FSD_FLOORKICK_LIMIT 1
#endif

#ifndef FSD_RANDOMIZER
#define FSD_RANDOMIZER FST_RAND_BAG7
#endif

#ifndef FSD_INFINITE_READY_GO_HOLD
#define FSD_INFINITE_READY_GO_HOLD false
#endif

#ifndef FSD_READY_PHASE_LENGTH
#define FSD_READY_PHASE_LENGTH 833
#endif

#ifndef FSD_SOUND_ON_BAD_FINESSE
#define FSD_SOUND_ON_BAD_FINESSE false
#endif

#ifndef FSD_ARE_CANCELLABLE
#define FSD_ARE_CANCELLABLE false
#endif

#ifndef FSD_GO_PHASE_LENGTH
#define FSD_GO_PHASE_LENGTH 833
#endif

#ifndef FSD_NEXT_PIECE_COUNT
#define FSD_NEXT_PIECE_COUNT 4
#endif

#ifndef FSD_ONE_SHOT_SOFT_DROP
#define FSD_ONE_SHOT_SOFT_DROP false
#endif

#ifndef FSD_GOAL
#define FSD_GOAL 40
#endif

#ifndef FSD_KEY_ROTL
#define FSD_KEY_ROTL "z"
#endif

#ifndef FSD_KEY_ROTR
#define FSD_KEY_ROTR "x"
#endif

#ifndef FSD_KEY_ROTH
#define FSD_KEY_ROTH "a"
#endif

#ifndef FSD_KEY_LEFT
#define FSD_KEY_LEFT "left"
#endif

#ifndef FSD_KEY_RIGHT
#define FSD_KEY_RIGHT "right"
#endif

#ifndef FSD_KEY_DOWN
#define FSD_KEY_DOWN "down"
#endif

#ifndef FSD_KEY_UP
#define FSD_KEY_UP "space"
#endif

#ifndef FSD_KEY_HOLD
#define FSD_KEY_HOLD "c"
#endif

#ifndef FSD_KEY_QUIT
#define FSD_KEY_QUIT "q"
#endif

#ifndef FSD_KEY_RESTART
#define FSD_KEY_RESTART "rshift"
#endif

#endif // FS_DEFAULT_H

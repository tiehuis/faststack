///
// fsEngine.h
// ==========
//
// Header file for the FastStack engine.
//
// The engine is mostly opaque to an outside user. A number of functions are
// provided which provide some convenience when performing certain tasks.
//
// Notes:
//  * Rename this to FSEngine.h and fsTypes to fsCore.h
//
// Naming
// ------
// We use the following prefixes for enums.
//
//  FST_* - FastStack Type
//  FSS_* - FastStack State
///

#ifndef FS_H
#define FS_H

#include "fsConfig.h"
#include "fsControl.h"
#include "fsLog.h"
#include "fsRand.h"
#include "fsOption.h"
#include "fsTypes.h"
#include "fsRotation.h"

#include <stdbool.h>
#include <string.h>

///
// Convert an arbitrary index type (e.g. FST_SE_*) into its corresponding flag
// value (FST_SE_FLAG_*).
#define FS_TO_FLAG(x) (1 << (x))

enum SoundEffectIndex {
    FST_SE_GAMEOVER,
    FST_SE_READY,
    FST_SE_GO,
    FST_SE_IPIECE,
    FST_SE_JPIECE,
    FST_SE_LPIECE,
    FST_SE_OPIECE,
    FST_SE_SPIECE,
    FST_SE_TPIECE,
    FST_SE_ZPIECE,
    FST_SE_MOVE,
    FST_SE_ROTATE,
    FST_SE_HOLD,
    FST_SE_ERASE1,
    FST_SE_ERASE2,
    FST_SE_ERASE3,
    FST_SE_ERASE4,
    FST_SE_COUNT
};

/// Types of sound effects
enum SoundEffect {
    FST_SE_FLAG_GAMEOVER = (1 << FST_SE_GAMEOVER),
    FST_SE_FLAG_READY    = (1 << FST_SE_READY),
    FST_SE_FLAG_GO       = (1 << FST_SE_GO),
    FST_SE_FLAG_IPIECE   = (1 << FST_SE_IPIECE),
    FST_SE_FLAG_JPIECE   = (1 << FST_SE_JPIECE),
    FST_SE_FLAG_LPIECE   = (1 << FST_SE_LPIECE),
    FST_SE_FLAG_OPIECE   = (1 << FST_SE_OPIECE),
    FST_SE_FLAG_SPIECE   = (1 << FST_SE_SPIECE),
    FST_SE_FLAG_TPIECE   = (1 << FST_SE_TPIECE),
    FST_SE_FLAG_ZPIECE   = (1 << FST_SE_ZPIECE),
    FST_SE_FLAG_MOVE     = (1 << FST_SE_MOVE),
    FST_SE_FLAG_ROTATE   = (1 << FST_SE_ROTATE),
    FST_SE_FLAG_HOLD     = (1 << FST_SE_HOLD),
    FST_SE_FLAG_ERASE1   = (1 << FST_SE_ERASE1),
    FST_SE_FLAG_ERASE2   = (1 << FST_SE_ERASE2),
    FST_SE_FLAG_ERASE3   = (1 << FST_SE_ERASE3),
    FST_SE_FLAG_ERASE4   = (1 << FST_SE_ERASE4)
};

///
// Locking System type.
///,
enum LockStyle {
    /// Lock delay is reset only on entry of a new piece.
    FST_LOCK_ENTRY,

    /// Lock delay is reset on any downwards movement.
    FST_LOCK_STEP,

    /// Lock delay is reset on any **successful** movement.
    FST_LOCK_MOVE
};

///
// Initial Action type.
///
enum InitialActionType {
    /// IHS/IRS is disabled.
    FST_IA_NONE,

    /// IHS/IRS can be triggered from a last frame action
    FST_IA_PERSISTENT,

    /// IHS/IRS must get a new event to trigger
    FST_IA_TRIGGER
};

///
// All possible game states.
///
enum GameState {
    /// Occurs whilst 'READY' is displayed
    FSS_READY,

    /// Occurs whilst 'GO' is displayed
    FSS_GO,

    /// Occurs when a piece has nothing beneath it.
    FSS_FALLING,

    /// Occurs when a piece has hit the top of the stack/floor.
    FSS_LANDED,

    /// Occurs when waiting for a new piece to spawn (entry delay)
    FSS_ARE,

    /// Occurs when a new piece needs to be spawned. This occurs instantly.
    FSS_NEW_PIECE,

    /// (unused) Occurs when a line clear is occurring.
    FSS_LINES,

    /// Occurs when a user-specified quit action occurred.
    FSS_QUIT,

    /// Occurs when the user lost (topped out).
    FSS_GAMEOVER,

    /// Occurs when the user restarts.
    FSS_RESTART,

    /// Unknown state
    FSS_UNKNOWN
};

///
// A single FastStack game instance.
//
// Stores all internal variables and options pertaining to a field.
// Values can be broken down into one of three classes.
//
//  * Internal Status (@I)
//      Only used internally and never required to be read by a platform.
//
//  * External Status (@E)
//      Calculated internally by the engine, but expected to be read by a
//      user.
//
//  * Fixed Option (@O)
//      Can be set by the user. Typically unsafe to change during execution.
//
//  We document which of the following variables belongs to which class. These
//  are only guidelines and there may be cases where we need to break the
//  following visibility rules.
//
//  Note: ANy 'Constraints' should always be true at any point in time.
///
typedef struct FSEngine {
    /// @E: Current field state.
    FSBlock b[FS_MAX_HEIGHT][FS_MAX_WIDTH];

    /// @O: Current field width.
    //
    //  * Constraints
    //      * fieldWidth < FS_MAX_WIDTH
    i8 fieldWidth;

    /// @O: Current field height.
    //
    //  * Constraints
    //      * fieldHeight < FS_MAX_HEIGHT
    i8 fieldHeight;

    /// @0: Number of hidden rows.
    //
    // These are not extra rows, but rather count how many field rows are
    // treated as hidden.
    i8 fieldHidden;

    /// @E: Next available pieces.
    FSBlock nextPiece[FS_PREVIEW_MAX];

    /// @I: Current random state context.
    FSRandCtx randomContext;

    /// @I: Buffer for calculating next pieces.
    FSBlock randBuf[FS_RAND_BUFFER_LEN];

    /// @I: Index for `randBuf`
    int randBufIndex;

    /// @O: The way we should handle Initial Actions.
    i8 initialActionStyle;

    /// @E: Current sound effects to be played this frame.
    u32 se;

    /// @E: Current pieces type.
    FSBlock piece;

    /// @E: Current pieces x position.
    i8 x;

    /// @E: Current pieces y position.
    i8 y;

    /// @I: Actual y position with greater precision.
    //
    // To calculate soft drop and gravity we need more precision than an
    // integer can provide.
    //
    //  * Constraints
    //      * y == (float) actualY
    float actualY;

    /// @I: Greatest 'y' the current piece can exist at without a collision.
    i8 hardDropY;

    /// @E: Current pieces rotation state.
    i8 theta;

    /// @I: Current Initial Rotation status (set in ARE)
    i8 irsAmount;

    /// @I: Current Initial Hold status (set in ARE)
    bool ihsFlag;

    /// @O: How many blocks a piece moves per ms.
    i8 dasSpeed;

    /// @O: Number of ms a key must be held before repeated movement.
    i32 dasDelay;

    /// @E: Number of wasted movements have occurred during the games
    //      lifetime.
    i32 finesse;

    /// @I: Number of directional movements have been performed during this
    //     pieces lifetime.
    i32 finessePieceDirection;

    /// @I: Number of rotational movements have been performed during this
    //     pieces lifetime.
    i32 finessePieceRotation;

    /// @O: Milliseconds between each game logic update.
    i8 msPerTick;

    /// @O: How many game ticks occur per draw update.
    i32 ticksPerDraw;

    /// @O: Length in ms that ARE should take.
    i32 areDelay;

    /// @I: Counter for ARE.
    i32 areTimer;

    /// @O: Can ARE be cancelled by input
    bool areCancellable;

    /// @E: Actual game length using a high precision timer.
    //
    // The game length is usually calculated as 'msPerTick * totalTicks' but
    // this is potentially inaccurate up to (+-msPerTick). 'actualTimer' acts
    // as a reliable source to ensure the game was played at the correct speed.
    //
    // This is calculated **only** on game finish.
    i32 actualTime;

    /// @I: Generic counter for multi-tick usage.
    i32 genericCounter;

    /// @E: Number of ticks that have elapsed during this game.
    i32 totalTicks;

    /// @O: Current lock reset style in use.
    i8 lockStyle;

    /// @O: Length in ms that it should take to lock a piece.
    i32 lockDelay;

    /// @I: Counter for locking.
    i32 lockTimer;

    /// @O: Maximum number of floorkicks allowed per piece.
    i8 floorkickLimit;

    /// @I: Count for how many floorkicks have occured.
    i8 floorkickCount;

    /// @O: Should soft drop be a single shot on each key press.
    bool oneShotSoftDrop;

    /// @O: Current rotation system being used.
    i8 rotationSystem;

    /// @O: How many blocks a piece will fall by every ms.
    float gravity;

    /// @O: How many blocks a piece will fall by every ms when soft dropping.
    float softDropGravity;

    /// @E: Current state of the internal engine.
    i8 state;

    /// @E: State of the game during the last frame.
    i8 lastState;

    /// @I: Key input applied during the last logic update.
    FSInput lastInput;

    /// @O: Current randomizer in play. */
    i8 randomizer;

    /// @I: Randomizer seed
    u32 seed;

    /// @I: The randomizer in use during the last game update.
    //
    // Used to determine if reinitialization of a randomizer is required.
    // This allows one to alter than randomizer mid-game.
    i8 lastRandomizer;

    /// @O: How long the "Ready" phase countdown should last in ms
    i32 readyPhaseLength;

    /// @O: How long the "Go" phase countdown should last in ms
    i32 goPhaseLength;

    /// @O: Whether infinite hold is allowed during pre-game.
    bool infiniteReadyGoHold;

    /// @O: Number of preview pieces displayed.
    i8 nextPieceCount;

    /// @I: Whether a hold can be performed.
    bool holdAvailable;

    /// @E: Current piece we are holding.
    FSBlock holdPiece;

    /// @E: Number of cleared lines during the games lifetime
    i32 linesCleared;

    /// @E: Number of blocks placed during the games lifetime.
    i32 blocksPlaced;

    /// @O: Target number of lines to clear during this game.
    i32 goal;
} FSEngine;

///
// A generic view of a games components.
//
// The 'FSEngine' instance does not handle all the components, such as input.
// This view encapsulates all these components into one structure.
///
typedef struct FSView {
    /// Current game instance.
    FSEngine *game;

    /// Current input state.
    FSControl *control;

    /// Number of draw requests made during this views lifetime.
    i32 totalFramesDrawn;
} FSView;

///
// Clear the specified game instance.
//
// This only resets internal variables and will not overwrite any user specified
// options. This is suitable to call when a new game is wanted to be started
// without reloading an options file/keeping track of options elsewhere.
//
//  * FSEngine *f
//      The instance to clear.
///
void fsGameReset(FSEngine *f);

///
// Initialize a game instance.
//
// This will reset all internal variables and also set options to their
// default values.
///
void fsGameInit(FSEngine *f);

///
// Perform a single game update.
//
//  * FSEngine *f
//      The instance to update
//
//  * const FSInput *i
//      The input for the instance to compute.
///
void fsGameTick(FSEngine *f, const FSInput *i);

///
// Convert the specified into its individual blocks.
//
//  * const FSEngine *f
//      The instance which options are used
//
//  * i8x2 *dst
//      The destination buffer to store the pieces in.
//
//      Note: **must** be greater than or equal to FS_NBP in size.
//
//  * i8 piece
//      Type of piece to generate.
//
//  * int x
//      X coordinate of the piece
//
//  * int y
//      Y coordinate of the piece
//
//  * int theta
//      Rotation state of the piece.
//
///
void fsPieceToBlocks(const FSEngine *f, i8x2 dst[static FS_NBP],
                     i8 piece, int x, int y, int theta);

#endif // FS_H

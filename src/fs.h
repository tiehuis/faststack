// fs.h
//
// Main header file for FastStack engine.

#ifndef FS_H
#define FS_H

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "fsConfig.h"
#include "fsControl.h"
#include "fsTypes.h"

// The following are fixed. They could change in the future though and it
// better displays intent. The identifiers are purposely short because of
// their prevalence. This is considered fine since everything else has clear
// identifiers.

// Number of piece types
#define FS_NPT 7

// Number of Rotation Systems
#define FS_NRS 5

// Number of Piece Rotations
#define FS_NPR 4

// (Max) Number of Blocks in a Piece
#define FS_NBP 4

enum BlockType {
    FS_I,
    FS_J,
    FS_L,
    FS_O,
    FS_S,
    FS_T,
    FS_Z,
    FS_NONE
};

enum RandomizerType {
    FSRAND_UNDEFINED,
    FSRAND_SIMPLE,
    FSRAND_NOSZO_BAG7
};

enum RotationSystemType {
    FSROT_SIMPLE,
    FSROT_SRS,
    FSROT_ARIKA_SRS,
    FSROT_TGM12,
    FSROT_DTET
};

enum RotationAmount {
    FSROT_CLOCKWISE = 1,
    FSROT_ANTICLOCKWISE = -1,
    FSROT_HALFTURN = 2
};

enum LockStyle {
    FSLOCK_ENTRY,
    FSLOCK_STEP,
    FSLOCK_MOVE
};

enum GameState {
    FSS_FALLING,
    FSS_LANDED,
    FSS_ARE,
    FSS_NEW_PIECE,
    FSS_LINES_FALLING,
    FSS_LINES,
    FSS_QUIT,
    FSS_GAMEOVER
};

typedef FSInt3 WallkickTable[FS_NPR][FS_MAX_KICK_LEN];

// Specific rotation systems are defined in fsTables.c.
// NOTE: Should we allow reading dynamic rotation systems from file?
typedef struct FSRotationSystem {
    // Base movements offsets
    FSInt entryOffset[FS_NPT];

    // Base rotation offsets
    FSInt entryTheta[FS_NPT];

    // Pointers into the kick table for each piece
    FSInt kicksL[FS_NPT];
    FSInt kicksR[FS_NPT];
    FSInt kicksH[FS_NPT];

    // Actual kick values for each table.
    // A kick value is comprised of an x, y and extra field, which
    // can specify special cases that are then handled individually in
    // the rotation logic.
    WallkickTable kickTables[FS_MAX_NO_OF_WALLKICK_TABLES];
} FSRotationSystem;

extern const FSRotationSystem *rotationSystems[FS_NRS];
extern const WallkickTable emptyWallkickTable;

// The main game structure. Stores all current game state/options excluding
// timing based input, which is abstracted to FSControl.
typedef struct FSGame {
    // Board state
    FSBlock b[FS_MAX_HEIGHT][FS_MAX_WIDTH];

    // The width is variable but less than FS_MAX_WIDTH
    FSInt fieldWidth;

    // The height is variable but less than FS_MAX_HEIGHT
    FSInt fieldHeight;

    // A buffer of all the upcoming pieces
    FSBlock nextPiece[FS_PREVIEW_MAX];

    // Internal buffer used by all randomizer states
    FSBlock randomInternal[FS_RAND_BUFFER_LEN];

    // Index into randomInternal, useful for a bag
    int randomInternalIndex;

    // The current piece type
    FSBlock piece;

    // Current piece x
    FSInt x;

    // Current piece y
    FSInt y;

    // The actual y with fractional portion.
    // This is required since gravity is not strictly integer based.
    // y will ALWAYS be (int) actualY
    float actualY;

    // Highest y which the current piece can lie
    FSInt hardDropY;

    // Current block rotation state
    FSInt theta;

    // Current finesse (wasted movement count)
    FSLong finesse;

    // How many movement keypresses during this pieces lifetime
    FSLong finessePieceDirection;

    // How many rotation keypresses during this pieces lifetime
    FSLong finessePieceRotation;

    // NOTE: This should be microsecond granularity to provide as accurate
    // as possible timings.
    // Time taken to perform a game update in ms (default 16)
    FSInt msPerTick;

    // How long each draw tick should take.
    FSLong msPerDraw;

    // How long ARE should last
    FSLong areDelay;

    // Current time we have been in ARE
    FSLong areTimer;

    // Actual time that passed according to the specified clock.
    // NOTE: This is solely to ensure that the clock was accurate, but can be
    // used for high-speed timing as another reliable measure.
    FSLong actualTime;

    // Total ticks elapsed
    FSLong totalTicks;

    // Which lock style we are using
    FSInt lockStyle;

    // Lock delay (in ms)
    FSLong lockDelay;

    // How long we have been locking for (in ticks)
    FSLong lockTimer;

    // Usually SRS
    FSInt rotationSystem;

    // Current gravity
    float gravity;

    // Soft drop gravity
    float softDropGravity;

    // Current game state
    FSInt state;

    // Last input movement found
    FSInput lastInput;

    // We remove the requirement to explicitly initialize randomizer
    // using thread-local variables and a last flag state.
    FSInt lastRandomizer;
    FSInt randomizer;

    // Can we currently hold?
    bool holdAvailable;

    // What is the current hold piece? (-1 if none)
    FSBlock holdPiece;

    // How many lines have been cleared?
    FSLong linesCleared;

    // How many blocks have been placed?
    FSLong blocksPlaced;

    // The target goal for this line race
    FSLong goal;
} FSGame;

// A generic wrapper representing a current view of a game.
typedef struct FSView {
    // The current internal game configuration/state
    FSGame *game;

    // The current input state
    FSControl *control;

    // How many frames this view has drawn
    FSLong totalFramesDrawn;
} FSView;

// Clear the field instance.
void fsGameClear(FSGame *f);

// Perform logic for a single game tick.
void fsGameDoTick(FSGame *f, const FSInput *i);

// Return a new random piece using the current randomizer.
FSBlock fsNextRandomPiece(FSGame *f);

// Convert the specified piece into its component blocks.
void fsPieceToBlocks(const FSGame *f, FSInt2 *dst, FSInt piece, int x, int y, int theta);

#endif

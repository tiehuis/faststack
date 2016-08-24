///
// `fs.c`
//
// FastStack Engine implementation.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "fs.h"
#include "fsDefault.h"
#include "fsInternal.h"

///
// @impl in `fsRand.c`
//
// Return the next randomly generated piece.
FSBlock fsNextRandomPiece(FSGame *f);
// Initialize a randomizer instance with a seed
void fsRandSeed(FSRandCtx *ctx, uint32_t seed);


///
// Static piece offsets.
//
// These map to SRS rotation by default. Alternate rotation systems
// are specific in 'fsTables.c' by customising the default wallkick
// test to account for the differences.
//
// This complicates wallkicks for some otherwise simple rotations, but
// in my experience is cleaner than implementing different base offsets.
static const FSInt2 pieceOffsets[FS_NPT][FS_NPR][FS_NBP] = {
    [FS_I] = {
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
        {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
        {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}}
    },
    [FS_J] = {
        {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 0}},
        {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
        {{0, 2}, {1, 0}, {1, 1}, {1, 2}}
    },
    [FS_L] = {
        {{0, 1}, {1, 1}, {2, 0}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
        {{0, 1}, {0, 2}, {1, 1}, {2, 1}},
        {{0, 0}, {1, 0}, {1, 1}, {1, 2}}
    },
    [FS_O] = {
        {{1, 0}, {1, 1}, {2, 0}, {2, 1}},
        {{1, 0}, {1, 1}, {2, 0}, {2, 1}},
        {{1, 0}, {1, 1}, {2, 0}, {2, 1}},
        {{1, 0}, {1, 1}, {2, 0}, {2, 1}}
    },
    [FS_S] = {
        {{0, 1}, {1, 0}, {1, 1}, {2, 0}},
        {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
        {{0, 2}, {1, 1}, {1, 2}, {2, 1}},
        {{0, 0}, {0, 1}, {1, 1}, {1, 2}}
    },
    [FS_T] = {
        {{0, 1}, {1, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 1}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 1}},
        {{0, 1}, {1, 0}, {1, 1}, {1, 2}}
    },
    [FS_Z] = {
        {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
        {{1, 1}, {1, 2}, {2, 0}, {2, 1}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
        {{0, 1}, {0, 2}, {1, 0}, {1, 1}}
    }
};

/* Specifies the value stored in each cell. Not currently utilized much. */
const FSInt pieceColors[7] = {
    0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
};

///
// Return the next preview piece from the queue.
///
static FSBlock nextPreviewPiece(FSGame *f)
{
    const FSBlock newPiece = fsNextRandomPiece(f);

    if (f->nextPieceCount == 0) {
        return newPiece;
    }
    else {
        const FSBlock pendingPiece = f->nextPiece[0];
        memmove(f->nextPiece, f->nextPiece + 1, f->nextPieceCount - 1);
        f->nextPiece[f->nextPieceCount - 1] = newPiece;
        return pendingPiece;
    }
}

void fsGameClear(FSGame *f)
{
    // Handle all required zero values here, there are some special cases
    memset(f, 0, sizeof(FSGame));

    // Initialize game-specific random context
    fsRandSeed(&f->randomContext, time(NULL));

    // Signal that we are changing the randomizer and need to reinitialize
    f->lastRandomizer = FSRAND_UNDEFINED;

    // Set defaults
    f->fieldWidth = FSD_FIELD_WIDTH;
    f->fieldHeight = FSD_FIELD_HEIGHT;
    f->msPerTick = FSD_MS_PER_TICK;
    f->ticksPerDraw = FSD_TICKS_PER_DRAW;
    f->areDelay = FSD_ARE_DELAY;
    f->initialActionStyle = FSD_INITIAL_ACTION_STYLE;
    f->lockStyle = FSD_LOCK_STYLE;
    f->lockDelay = FSD_LOCK_DELAY;
    f->rotationSystem = FSD_ROTATION_SYSTEM;
    f->gravity = FSD_GRAVITY;
    f->softDropGravity = FSD_SOFT_DROP_GRAVITY;
    f->randomizer = FSD_RANDOMIZER;
    f->infiniteReadyGoHold = FSD_INFINITE_READY_GO_HOLD;
    f->nextPieceCount = FSD_NEXT_PIECE_COUNT;
    f->areCancellable = FSD_ARE_CANCELLABLE;
    f->readyPhaseLength = FSD_READY_PHASE_LENGTH;
    f->goPhaseLength = FSD_GO_PHASE_LENGTH;
    f->goal = FSD_GOAL;

    // Internal defaults
    f->state = FSS_READY;
    f->holdAvailable = true;
    f->holdPiece = FS_NONE;

    // We only initialize the current piece after the ready/go is complete.
    // This ensures we don't render it to the field until we begin.
    f->piece = FS_NONE;
    for (int i = 0; i < f->nextPieceCount; ++i) {
        f->nextPiece[i] = fsNextRandomPiece(f);
    }
}

// Could extend to store colour here as well.
// If we want to allow specific piece colouring as an option. Not essential
// right now, manage in frontend.
void fsPieceToBlocks(const FSGame *f, FSInt2 *dst, FSInt piece, int x, int y, int theta)
{
    // A rotation system could be offset
    const FSRotationSystem *rs = rotationSystems[f->rotationSystem];
    const int calcTheta = (theta + rs->entryTheta[piece]) & 3;

    for (int i = 0; i < FS_NBP; ++i) {
        dst[i].x = pieceOffsets[piece][calcTheta][i].x + x;
        dst[i].y = pieceOffsets[piece][calcTheta][i].y + y;
    }
}

///
// Is the specified location on the field occupied?
//
// If the coordinates are outside the field, false is returned.
static bool isOccupied(const FSGame *f, int x, int y)
{
    if (x < 0 || x >= f->fieldWidth || y < 0 || y >= f->fieldHeight)
        return true;

    return f->b[y][x] > 1;
}

///
// Does the current piece collide at the specified coordinates/rotation.
static bool isCollision(const FSGame *f, int x, int y, int theta)
{
    FSInt2 blocks[FS_NBP];

    fsPieceToBlocks(f, blocks, f->piece, x, y, theta);

    for (int i = 0; i < FS_NBP; ++i) {
        if (isOccupied(f, blocks[i].x, blocks[i].y)) {
            return true;
        }
    }
    return false;
}

// Lock the active piece to the playfield and perform any required
// routines.
// NOTE: Finesse is currently a bit off.
static void lockPiece(FSGame *f)
{
    FSInt2 blocks[FS_NBP];
    fsPieceToBlocks(f, blocks, f->piece, f->x, f->y, f->theta);
    f->blocksPlaced += 1;

    for (int i = 0; i < FS_NBP; ++i) {
        f->b[blocks[i].y][blocks[i].x] = pieceColors[f->piece];
    }

    // Compute the finesse of this piece and add if we have wasted keypresses.
    // We use a simple algorithm. Every location can be reached in 2 presses most
    // (under DAS), so use this as an upper bound. This means it isn't 100%
    // accurate for close values, but these are not the problem areas most people
    // have. Also, this assumes the SRS rotation system, or any system where
    // every location is reachable with 2 presses.
    int wastedDirection = f->finessePieceDirection > 2 ? f->finessePieceDirection - 2 : 0;

    // How many movements to get to each rotation optimally (excluding 180)
    const int fLook[FS_NPR] = { 0, 1, 2, 1 };

    // We should handle overhangs here to ideally
    // We do not count finesse for 180 degree rotation by default (but could)

    // O piece should never be rotated
    int wastedRotation;
    if (f->piece == FS_O) {
        wastedRotation = f->finessePieceRotation > fLook[f->theta]
                            ? f->finessePieceRotation - fLook[f->theta]
                            : 0;
    }
    else {
        wastedRotation = f->finessePieceRotation;
    }

    f->finesse += wastedDirection + wastedRotation;
}

///
// Generate a new piece and 'spawn' it to the field.
static void newPiece(FSGame *f)
{
    // NOTE: Should use wallkick entryOffset here probably, and entryTheta?
    // Else we are maintaining the current where we map only when the blocks
    // themselves are generated. Think about this.

    f->x = f->fieldWidth / 2 - 1;
    f->y = 0;
    f->actualY = 0;
    f->theta = 0;
    f->lockTimer = 0;
    f->finessePieceRotation = 0;
    f->finessePieceDirection = 0;
    f->piece = nextPreviewPiece(f);
    f->holdAvailable = true;
}

///
// Try to rotate the current piece in the specified direction using the
// existing rotation system.
static bool doRotate(FSGame *f, FSInt direction)
{
    // Get the appropriate table and theta
    FSInt newDir = (f->theta + 4 + direction) & 3;
    const FSRotationSystem *rs = rotationSystems[f->rotationSystem];

    FSInt tableNo;
    switch (direction) {
      case FSROT_CLOCKWISE:
        tableNo = rs->kicksR[f->piece];
        break;
      case FSROT_ANTICLOCKWISE:
        tableNo = rs->kicksL[f->piece];
        break;
      case FSROT_HALFTURN:
        tableNo = rs->kicksH[f->piece];
        break;
      default:
        abort();
    }

    const WallkickTable *table = tableNo >= 0
                                    ? &rs->kickTables[tableNo]
                                    : &emptyWallkickTable;

    for (int k = 0; k < FS_MAX_KICK_LEN; ++k) {
        // NOTE: Check which theta we should be using here
        // We need to reverse the kick rotation here
        const FSInt3 kickData = (*table)[f->theta][k];

        // Early break if no more kicks are available
        if (kickData.z == WK_END) {
            break;
        }

        int kickX = kickData.x + f->x;
        int kickY = kickData.y + f->y;

        if (!isCollision(f, kickX, kickY, newDir)) {
            f->y = kickY;
            f->x = kickX;
            f->theta = newDir;
            return true;
        }
    }

    return false;
}

///
// Apply the specified gravity. To the piece.
static void doPieceGravity(FSGame *f, FSInt gravity)
{
    f->actualY += (f->msPerTick * f->gravity) + gravity;

    // If we will go beyond the bottom of the screen we have landed
    if (f->actualY >= f->hardDropY) {
        f->actualY = f->hardDropY;
        f->y = f->hardDropY;

        // Change the state accordingly
        if (f->state == FSS_FALLING) {
            f->state = FSS_LANDED;
        }
    }
    else {
        // Check if we have moved a square and reset lock delay if so
        if ((f->lockStyle == FSLOCK_STEP || f->lockStyle == FSLOCK_MOVE) &&
                (int) f->actualY > f->y)
            f->lockTimer = 0;

        f->y = (FSInt) f->actualY;
        f->state = FSS_FALLING;
    }
}

///
// Find all full rows and clear them, moving upper rows down.
// The algorithm used is as follows:
//
// 1. Check each row, setting a flag if it is full
// 2. Walk through each row, if the flag was set copy it, else skip
// 3. Clear remaining upper rows
//
// This requires only two passes of the data, and at worst copying of
// fieldHeight - 1 rows.
///
static FSInt clearLines(FSGame *f)
{
    // This limits the maximum field height to 32
    FSBits foundLines = 0;
    FSInt filledLineCount = 0;

    for (int y = 0; y < f->fieldHeight; ++y) {
        for (int x = 0; x < f->fieldWidth; ++x) {
            if (f->b[y][x] == 0) {
                // Push lines on so bottom row is LSB
                goto next_row;
            }
        }
        foundLines |= 1;
        filledLineCount += 1;

next_row:
        foundLines <<= 1;
    }

    // We need to not shift the last row else we will be off by one.
    foundLines >>= 1;

    // Perform a second pass, copying rows down the stack directly into place
    int dst = f->fieldHeight - 1;
    for (int src = dst; src >= 0; --src, foundLines >>= 1) {
        if (foundLines & 1) {
            continue;
        }

        if (src != dst) {
            memcpy(f->b[dst], f->b[src], sizeof(FSBlock) * f->fieldWidth);
        }

        --dst;
    }

    for (int i = 0; i < filledLineCount; ++i) {
        memset(f->b[i], 0, sizeof(FSBlock) * f->fieldWidth);
    }

    return filledLineCount;
}

// Compute the maximum y this piece can be before colliding in its current
// rotation state.
void updateHardDropY(FSGame *f)
{
    int y = f->y;
    while (!isCollision(f, f->x, y, f->theta)) {
        y += 1;
    }

    f->hardDropY = y - 1;
}

///
// Attempt to hold the piece, returning if a hold was successful.
///
static bool tryHold(FSGame *f)
{
    if (f->holdAvailable) {
        f->holdAvailable = false;
        if (f->holdPiece == FS_NONE) {
            f->holdPiece = f->piece;
            newPiece(f);
            f->holdAvailable = false;
        }
        else {
            // Abstract into new piece of type theta
            f->x = f->fieldWidth / 2 - 1;
            f->y = 0;
            f->actualY = 0;
            f->theta = 0;
            f->lockTimer = 0;

            // Swap block types
            FSBlock t = f->holdPiece;
            f->holdPiece = f->piece;
            f->piece = t;
        }

        updateHardDropY(f);
        f->se |= FSSE_HOLD;
        return true;
    }
    else {
        return false;
    }
}

///
// A single game tick.
//
// This is just a state machine which is repeatedly called from the main
// game loop. We do not want a 1 frame delay for some actions so we allow
// some to run 'instantly'.
///
void fsGameTick(FSGame *f, const FSInput *i)
{
    FSInt distance;
    bool moved = false, rotated = false;

    // Always clear any previous sound effects.
    f->se = 0;

    // Store the input encountered so we can debug it on the
    // frontend if required.
    f->lastInput = *i;

beginTick:
    switch (f->state) {
        // We still want to handle hold during the start phase
      case FSS_READY:
      case FSS_GO:
        if ((i->extra & FSI_HOLD) && f->holdAvailable) {
            // Hold action pre-game is slightly different since we do not
            // actually have a piece here yet.
            //
            // Move our pending piece directly into hold and shift
            // preview segment.
            f->holdPiece = nextPreviewPiece(f);
            f->se |= FSSE_HOLD;

            if (!f->infiniteReadyGoHold)
                f->holdAvailable = false;
        }

        if (f->genericCounter == 0) {
            f->se |= FSSE_READY;
        }

        if (f->genericCounter == TICKS(f->readyPhaseLength)) {
            f->se |= FSSE_GO;
            f->state = FSS_GO;
        }

        // This cannot be an `else if` since goPhaseLength could be 0.
        if (f->genericCounter == TICKS(f->readyPhaseLength) + TICKS(f->goPhaseLength)) {
            f->state = FSS_NEW_PIECE;
        }

        // Return so we don't increment totalTicks and update game clock
        f->genericCounter++;
        return;

      case FSS_ARE:
        // Even if ARE is instant, we still want to check for IHS and IRS state.
        // This allows the following behaviour:
        //
        // Currently we should be able to have three different actions for an initial
        // action:
        //
        //  NONE - IRS/IHS disabled and not checked
        //  HELD - Allows input action to remain set from last piece
        //  HIT  - Requires a new input action to trigger (not implemented)
        //
        // If ARE can be cancelled then the action will occur on the next
        // frame with the piece already playable.
        // This may need some more tweaking since during fast play initial stack
        // far too easily.
        if (f->initialActionStyle == FSIA_PERSISTENT) {
            // Only check the current key state.
            // This is only dependent on the value on the final frame before the
            // piece spawns. Could adjust to allow any mid-ARE initial action to
            // stick till spawn.

            // We need an implicit ordering here so are slightly biased. May want to
            // give an option to adjust this ordering or have a stricter
            // order.
            if (i->currentKeys & VKEY_ROTR)
                f->irsAmount = FSROT_CLOCKWISE;
            else if (i->currentKeys & VKEY_ROTL)
                f->irsAmount = FSROT_ANTICLOCKWISE;
            else if (i->currentKeys & VKEY_ROTH)
                f->irsAmount = FSROT_HALFTURN;
            else
                f->irsAmount = FSROT_NONE;

            if (i->currentKeys & VKEY_HOLD)
                f->ihsFlag = true;
            else
                f->ihsFlag = false;
        }

        // Allow ARE to be skipped if any input is found. This is slightly laggy
        // on certain inputs and needs to be refined.
        if (f->areCancellable && (
                i->rotation != 0 ||
                i->movement != 0 ||
                i->gravity  != 0 ||
                i->extra    != 0 ||
                // We also need to check irs/ihs state since this is based solely
                // on new key state and may not be picked up
                f->ihsFlag || f->irsAmount
                )
        ) {
            f->areTimer = 0;
            f->state = FSS_NEW_PIECE;
            goto beginTick;
        }

        if (f->areTimer++ > TICKS(f->areDelay)) {
            f->areTimer = 0;
            f->state = FSS_NEW_PIECE;
            goto beginTick;
        }
        break;

      case FSS_NEW_PIECE:
        newPiece(f);

        // Apply IHS/IRS before checking lockout if this was triggered in ARE phase.
        if (f->irsAmount != FSROT_NONE) {
            doRotate(f, f->irsAmount);
        }
        if (f->ihsFlag) {
            // We still want to trigger the hold flag
            tryHold(f);
        }

        // Always reset irs/ihs on new piece
        f->irsAmount = FSROT_NONE;
        f->ihsFlag = false;

        // Check for lockout on spawn now IRS/IHS has been applied
        if (isCollision(f, f->x, f->y, f->theta)) {
            f->state = FSS_GAMEOVER;
            goto beginTick;
        }

        updateHardDropY(f);
        f->state = FSS_FALLING;
        break;

      case FSS_FALLING:
      case FSS_LANDED:
        // Handle hold
        if (i->extra & FSI_HOLD) {
            tryHold(f);
        }

        // Check finesse counters
        if (i->extra & FSI_FINESSE_DIRECTION) {
            f->finessePieceDirection += 1;
        }
        if (i->extra & FSI_FINESSE_ROTATION) {
            f->finessePieceRotation += 1;
        }

        if (i->rotation) {
            // We should allow for a true 180 or stepped 180
            if (doRotate(f, i->rotation)) {
                rotated = true;
            }
        }

        // Handle left/right movement
        distance = i->movement;
        for (; distance < 0; ++distance) {
            if (!isCollision(f, f->x - 1, f->y, f->theta)) {
                f->x -= 1;
                moved = true;
            }
        }

        for (; distance > 0; --distance) {
            if (!isCollision(f, f->x + 1, f->y, f->theta)) {
                f->x += 1;
                moved = true;
            }
        }

        if (moved || rotated) {
            if (moved)
                f->se |= FSSE_MOVE;
            if (rotated)
                f->se |= FSSE_ROTATE;

            updateHardDropY(f);

            // Note: Do we want O rotation to be always succesful?
            if (f->lockStyle == FSLOCK_MOVE)
                f->lockTimer = 0;
        }

        // Reset lock timer depending on the style

        // Next frame we clear lines if we landed with this piece
        doPieceGravity(f, i->gravity);

        // Check if we are now in a landed state. If a hard drop action was
        // input then bypass this and go directly to line clear.
        if ((i->extra & FSI_HARD_DROP) ||
                // We need to recheck state here since moving out of LANDED to
                // FALLING on the last lockDelay frame will lock the piece in
                // mid-air.
                (f->lockTimer > TICKS(f->lockDelay) && f->state == FSS_LANDED)) {
            f->state = FSS_LINES;
        }

        if (f->state == FSS_LANDED)
            f->lockTimer++;

        break;

      case FSS_LINES:
        // Clear the lines in 0 frames (instant) currently
        lockPiece(f);
        f->se |= (1 << (FSSEI_IPIECE + f->piece)); // Ordering is same for FSSE vs FS_PIECE
        f->piece = FS_NONE; // Invalidate piece so it is not drawn

        const int lines = clearLines(f);
        if (0 < lines && lines <= 4)
            f->se |= (FSSE_ERASE1 << (lines - 1));

        f->linesCleared += lines;
        f->state = f->linesCleared < f->goal ? FSS_ARE : FSS_GAMEOVER;
        goto beginTick;

      case FSS_GAMEOVER:
        f->se |= FSSE_GAMEOVER;
      case FSS_QUIT:
      default:
        break;
    }

    f->totalTicks += 1;
}

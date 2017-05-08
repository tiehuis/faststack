///
// engine.c
// ========
//
// faststack Engine implementation.
///

#include "default.h"
#include "engine.h"
#include "finesse.h"
#include "hiscore.h"
#include "internal.h"
#include "log.h"
#include "rotation.h"
#include "rand.h"

#include <string.h>
#include <stdlib.h>

/// Not currently utilized much.
const i8 pieceColors[FS_NPT] = {
    0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
};

///
// Return the next preview piece from the queue.
///
static FSBlock nextPreviewPiece(FSEngine *f)
{
    const FSBlock newPiece = fsNextRandomPiece(f);

    if (f->nextPieceCount == 0) {
        return newPiece;
    }

    const FSBlock pendingPiece = f->nextPiece[0];
    memmove(f->nextPiece, f->nextPiece + 1, f->nextPieceCount - 1);
    f->nextPiece[f->nextPieceCount - 1] = newPiece;
    return pendingPiece;
}

void fsGameReset(FSEngine *f)
{
    // We cannot simply memset the entire structure since we want to preserve
    // existing option (@O) values.
    //
    // Typically any added @I or @E piece needs to be added here as well.
    memset(f->b, 0, sizeof(f->b));
    memset(f->randBuf, 0, sizeof(f->randBuf));
    memset(&f->lastInput, 0, sizeof(f->lastInput));
    f->se = 0;
    f->irsAmount = 0;
    f->ihsFlag = false;
    f->replay = false;
    f->areTimer = 0;
    f->genericCounter = 0;
    f->totalKeysPressed = 0;
    f->totalTicks = 0;
    f->totalTicksRaw = 0;
    f->finesse = 0;
    f->lockTimer = 0;
    f->lastState = FSS_UNKNOWN;
    f->linesCleared = 0;
    f->blocksPlaced = 0;
    f->floorkickCount = 0;

    // The seed is assumed to be set by now by some external call!
    fsRandSeed(&f->randomContext, f->seed);

    // Signal that we are changing the randomizer and need to reinitialize
    f->lastRandomizer = FST_RAND_UNDEFINED;

    f->state = FSS_READY;
    f->holdAvailable = true;
    f->holdPiece = FS_NONE;

    // We do not generate a new piece here since we do not want to render it
    // during the ready/go phase.
    f->piece = FS_NONE;
    for (int i = 0; i < f->nextPieceCount; ++i) {
        f->nextPiece[i] = fsNextRandomPiece(f);
    }
}

///
// Initialize a game state.
//
// We want this seperate from standard initialization so we can reset a game
// without discarding user options.
void fsGameInit(FSEngine *f)
{
    f->fieldWidth = FSD_FIELD_WIDTH;
    f->fieldHeight = FSD_FIELD_HEIGHT;
    f->fieldHidden = FSD_FIELD_HIDDEN;
    f->msPerTick = FSD_MS_PER_TICK;
    f->ticksPerDraw = FSD_TICKS_PER_DRAW;
    f->areDelay = FSD_ARE_DELAY;
    f->dasSpeed = FSD_DAS_SPEED;
    f->dasDelay = FSD_DAS_DELAY;
    f->initialActionStyle = FSD_INITIAL_ACTION_STYLE;
    f->lockStyle = FSD_LOCK_STYLE;
    f->lockDelay = FSD_LOCK_DELAY;
    f->rotationSystem = FSD_ROTATION_SYSTEM;
    f->gravity = FSD_GRAVITY;
    f->softDropGravity = FSD_SOFT_DROP_GRAVITY;
    f->randomizer = FSD_RANDOMIZER;
    f->floorkickLimit = FSD_FLOORKICK_LIMIT;
    f->infiniteReadyGoHold = FSD_INFINITE_READY_GO_HOLD;
    f->nextPieceCount = FSD_NEXT_PIECE_COUNT;
    f->areCancellable = FSD_ARE_CANCELLABLE;
    f->readyPhaseLength = FSD_READY_PHASE_LENGTH;
    f->goPhaseLength = FSD_GO_PHASE_LENGTH;
    f->oneShotSoftDrop = FSD_ONE_SHOT_SOFT_DROP;
    f->goal = FSD_GOAL;

    fsGameReset(f);
}

///
// Return the set of `FS_NBP` locations the specified piece fills.
///
void fsGetBlocks(const FSEngine *f, i8x2 *dst, i8 piece, int x, int y, int theta)
{
    const FSRotationSystem *rs = rotationSystems[f->rotationSystem];
    const int calcTheta = (theta + rs->entryTheta[piece]) & 3;

    for (int i = 0; i < FS_NBP; ++i) {
        dst[i].x = pieceOffsets[piece][calcTheta][i].x + x;
        dst[i].y = pieceOffsets[piece][calcTheta][i].y + y;
    }
}

///
// Return whether the specified position is occupied by a block/field.
///
// If the coordinates are outside the field, false is returned.
static bool isOccupied(const FSEngine *f, int x, int y)
{
    if (x < 0 || x >= f->fieldWidth || y < 0 || y >= f->fieldHeight) {
        return true;
    }

    return f->b[y][x] > 1;
}

///
// Does the current piece collide at the specified coordinates/rotation.
///
static bool isCollision(const FSEngine *f, int x, int y, int theta)
{
    i8x2 blocks[FS_NBP];

    fsGetBlocks(f, blocks, f->piece, x, y, theta);

    for (int i = 0; i < FS_NBP; ++i) {
        if (isOccupied(f, blocks[i].x, blocks[i].y)) {
            return true;
        }
    }
    return false;
}

///
// Lock the current piece and perform post-piece specific routines.
///
static void lockPiece(FSEngine *f)
{
    i8x2 blocks[FS_NBP];
    fsGetBlocks(f, blocks, f->piece, f->x, f->y, f->theta);
    f->blocksPlaced += 1;

    for (int i = 0; i < FS_NBP; ++i) {
        f->b[blocks[i].y][blocks[i].x] = pieceColors[f->piece];
    }

    // Rotation in x field, Movement in y field
    const i8x2 optFinesse = fsMinimalFinesseCount(f->piece, f->x, f->theta);

    i8 rotation = f->pieceRotateCount - optFinesse.x;
    if (rotation < 0) { rotation = 0; }

    i8 movement = f->pieceMovePressCount - optFinesse.y;
    if (movement < 0) { movement = 0; }

    f->finesse += rotation + movement;
}

///
// Generate a new piece and 'spawn' it to the field.
///
static void newPiece(FSEngine *f)
{
    // NOTE: Should use wallkick entryOffset here probably, and entryTheta?
    // Else we are maintaining the current where we map only when the blocks
    // themselves are generated. Think about this.

    f->x = f->fieldWidth / 2 - 2;

    // We cannot spawn at 0, else Z, S cannot rotate under sega rules.
    // NOTE: Adjust hidden value on render side potentially to account.
    f->y = 1;
    f->actualY = fix(f->y);
    f->theta = 0;
    f->lockTimer = 0;
    f->pieceRotateCount = 0;
    f->pieceMovePressCount = 0;
    f->floorkickCount = 0;
    f->piece = nextPreviewPiece(f);
    f->holdAvailable = true;
}

///
// Check the arika LJT wallkick rotation special case.
//
// Return true if a rotation is valid with this field state and direction
// else false.
//
// Notes:
//  * These conditionals are a little hard to parse.
///
static bool wkCondArikaLJT(const FSEngine *f, int direction)
{
    // The following states are invalid if the x slot is occupied AND
    // the o slot is not occupied and traveling the specified
    switch (f->piece) {
      ///
      //  (cw)               (aw)
      //     o    x     x      o
      //   @@@   @@@   @     @x
      //    x@     @   @@@   @@@
      ///
      case FS_J:
        if (f->theta == 0 && (isOccupied(f, f->x + 1, f->y) ||
                (isOccupied(f, f->x + 1, f->y + 2) &&
                (direction == FST_ROT_CLOCKWISE ||
                 !isOccupied(f, f->x + 2, f->y))))) {
            return true;
        }
        if (f->theta == 2 && (isOccupied(f, f->x + 1, f->y) ||
                (isOccupied(f, f->x + 1, f->y + 1) &&
                (direction == FST_ROT_ANTICLOCKWISE ||
                 !isOccupied(f, f->x + 2, f->y))))) {
            return true;
        }
        break;

      ///
      //  (cw)        (aw)
      //   o      x    o      x
      //   @@@   @@@    x@     @
      //   @x    @     @@@   @@@
      ///
      case FS_L:
        if (f->theta == 0 && (isOccupied(f, f->x + 1, f->y) ||
                (isOccupied(f, f->x + 1, f->y + 2) &&
                (direction == FST_ROT_ANTICLOCKWISE ||
                 !isOccupied(f, f->x, f->y))))) {
            return true;
        }
        if (f->theta == 2 && (isOccupied(f, f->x + 1, f->y - 1) ||
                (isOccupied(f, f->x + 1, f->y) &&
                (direction == FST_ROT_CLOCKWISE ||
                 !isOccupied(f, f->x, f->y - 1))))) {
            return true;
        }
        break;

      ///
      //    x     x
      //    @    @@@
      //   @@@    @
      ///
      case FS_T:
        if (f->theta == 0 && isOccupied(f, f->x + 1, f->y)) {
            return true;
        }
        if (f->theta == 2 && isOccupied(f, f->x + 1, f->y - 1)) {
            return true;
        }
        break;

      default:
        break;
    }

    return false;
}

///
// Attempt to perform a rotation, returning whether the rotation succeeded.
///
static bool doRotate(FSEngine *f, i8 direction)
{
    i8 newDir = (f->theta + 4 + direction) & 3;
    const FSRotationSystem *rs = rotationSystems[f->rotationSystem];

    i8 tableNo;
    switch (direction) {
      case FST_ROT_CLOCKWISE:
        tableNo = rs->kicksR[f->piece];
        break;
      case FST_ROT_ANTICLOCKWISE:
        tableNo = rs->kicksL[f->piece];
        break;
      case FST_ROT_HALFTURN:
        tableNo = rs->kicksH[f->piece];
        break;
      default:
        abort();
    }

    const WallkickTable *table = tableNo >= 0
                                    ? &rs->kickTables[tableNo]
                                    : &emptyWallkickTable;

    // The `.z` field stores special wallkick flags.
    for (int k = 0; k < FS_MAX_KICK_LEN; ++k) {
        // NOTE: Check which theta we should be using here
        // We need to reverse the kick rotation here
        const i8x3 kickData = (*table)[f->theta][k];

        if (kickData.z == WK_END) {
            break;
        }

        // Handle special TGM123 rotation which is based on field state.
        if (kickData.z == WK_ARIKA_LJT && wkCondArikaLJT(f, direction)) {
            break;
        }

        int kickX = kickData.x + f->x;
        int kickY = kickData.y + f->y;

        if (!isCollision(f, kickX, kickY, newDir)) {
            // To determine a floorkick, we cannot just check the kickData.y
            // value since this may be adjusted for a different rotation system
            // (i.e. sega).
            //
            // We need to compute the difference between the current kickData.y
            // and the initial kickData.y instead to get an accurate reading.
            const int adjKickY = kickData.y - (*table)[f->theta][0].y;

            if (f->floorkickLimit && adjKickY < 0) {
                if (f->floorkickCount++ >= f->floorkickLimit) {
                    f->lockTimer = TICKS(f->lockDelay);
                }
            }

            // Preserve the fractional y drop during rotation to disallow
            // implicit lock reset.
            f->actualY = fix(kickY) + unfixfrc(f->actualY);
            f->y = kickY;
            f->x = kickX;
            f->theta = newDir;
            return true;
        }
    }

    return false;
}

///
// Apply the specified gravity to the piece.
//
// `gravity` is includes the calculated soft drop amount.
///
static void doPieceGravity(FSEngine *f, i8 gravity)
{
    f->actualY += (f->msPerTick * f->gravity) + fix(gravity);

    // If we overshoot the bottom of the field, fix to the lowest possible y
    // value the piece is valid at instead.
    if (f->actualY >= fix(f->hardDropY)) {
        f->actualY = fix(f->hardDropY);
        f->y = f->hardDropY;

        if (f->state == FSS_FALLING) {
            f->state = FSS_LANDED;
        }
    }
    else {
        if ((f->lockStyle == FST_LOCK_STEP || f->lockStyle == FST_LOCK_MOVE) &&
                unfixflr(f->actualY) > f->y) {
            f->lockTimer = 0;
        }

        f->y = unfixflr(f->actualY);
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
static i8 clearLines(FSEngine *f)
{
    // This effectively limits the maximum possible height to 32 rows.
    u32 foundLines = 0;
    i8 filledLineCount = 0;

    // 1: Mark filled rows.
    for (int y = 0; y < f->fieldHeight; ++y) {
        for (int x = 0; x < f->fieldWidth; ++x) {
            if (f->b[y][x] == 0) {
                goto next_row;
            }
        }
        foundLines |= 1;
        filledLineCount += 1;

next_row:
        foundLines <<= 1;
    }

    // Fix the last extra shift that isn't required
    foundLines >>= 1;

    // 2. Shift and replace filled rows.
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

///
// Recalculate and set the lowest valid Y position for the current piece.
///
void updateHardDropY(FSEngine *f)
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
static bool tryHold(FSEngine *f)
{
    if (f->holdAvailable) {
        f->holdAvailable = false;
        if (f->holdPiece == FS_NONE) {
            f->holdPiece = f->piece;
            newPiece(f);
            f->holdAvailable = false;
        }
        else {
            // NOTE: Abstract into new piece of type theta
            f->x = f->fieldWidth / 2 - 1;
            f->y = 1;
            f->actualY = fix(f->y);
            f->theta = 0;

            // NOTE: Utilize new piece here to reset variables
            //       consistently.
            f->floorkickCount = 0;

            FSBlock t = f->holdPiece;
            f->holdPiece = f->piece;
            f->piece = t;
        }

        updateHardDropY(f);
        f->se |= FST_SE_FLAG_HOLD;
        return true;
    }

    return false;
}

///
// Perform a single game tick.
//
// This is just a state machine which is repeatedly called from the main
// game loop. We do not want a 1 frame delay for some actions so we allow
// some to run 'instantly'.
///
void fsGameTick(FSEngine *f, const FSInput *i)
{
    i8 distance;
    bool moved = false, rotated = false;

    f->se = 0;
    f->totalTicksRaw++;

    // TODO: Remove lastInput since unused
    f->lastInput = *i;

    // Always handle restart/quit events at any time.
    if (i->extra & FST_INPUT_RESTART) {
        f->state = FSS_RESTART;
    }
    if (i->extra & FST_INPUT_QUIT) {
        f->state = FSS_QUIT;
    }

    // Always update the current piece finesse counters
    if (i->extra & FST_INPUT_FINESSE_ROTATE) {
        f->pieceRotateCount += 1;
    }
    if (i->extra & FST_INPUT_FINESSE_MOVE) {
        f->pieceMovePressCount += 1;
    }

    // Always count the number of new keys pressed
    f->totalKeysPressed += i->newKeysCount;

beginTick:
    switch (f->state) {
      case FSS_READY:
      case FSS_GO:

        // Ready, Go has has slightly different hold mechanics. Since we do not
        // yet have a piece we need to copy directly from the next queue to the
        // hold piece. Further, we can optionally hold as many times as we want
        // so need to discard the hold piece if required.
        if ((i->extra & FST_INPUT_HOLD) && f->holdAvailable) {
            f->holdPiece = nextPreviewPiece(f);
            f->se |= FST_SE_FLAG_HOLD;

            if (!f->infiniteReadyGoHold) {
                f->holdAvailable = false;
            }
        }

        if (f->genericCounter == 0) {
            f->se |= FST_SE_FLAG_READY;
        }

        if (f->genericCounter == TICKS(f->readyPhaseLength)) {
            f->se |= FST_SE_FLAG_GO;
            f->state = FSS_GO;
        }

        // This cannot be an `else if` since goPhaseLength could be 0.
        if (f->genericCounter == TICKS(f->readyPhaseLength) +
                                 TICKS(f->goPhaseLength)) {
            f->state = FSS_NEW_PIECE;
        }

        f->genericCounter++;

        // We need an explicit return here to avoid incrementing `totalTicks
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
        if (f->initialActionStyle == FST_IA_PERSISTENT) {
            // Only check the current key state.
            // This is only dependent on the value on the final frame before the
            // piece spawns. Could adjust to allow any mid-ARE initial action to
            // stick till spawn.

            // We need an implicit ordering here so are slightly biased. May want to
            // give an option to adjust this ordering or have a stricter
            // order.
            if (i->currentKeys & FST_VK_FLAG_ROTR) {
                f->irsAmount = FST_ROT_CLOCKWISE;
            }
            else if (i->currentKeys & FST_VK_FLAG_ROTL) {
                f->irsAmount = FST_ROT_ANTICLOCKWISE;
            }
            else if (i->currentKeys & FST_VK_FLAG_ROTH) {
                f->irsAmount = FST_ROT_HALFTURN;
            }
            else {
                f->irsAmount = FST_ROT_NONE;
            }

            if (i->currentKeys & FST_VK_FLAG_HOLD) {
                f->ihsFlag = true;
            }
            else {
                f->ihsFlag = false;
            }
        }

        if (f->areCancellable && (
                i->rotation != 0 ||
                i->movement != 0 ||
                i->gravity  != 0 ||
                i->extra    != 0 ||
                // We need to check ihs/irs since this is solely based on new
                // key state and otherwise may not be picked up.
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

        // Apply ihs/irs before checking lockout.
        if (f->irsAmount != FST_ROT_NONE) {
            doRotate(f, f->irsAmount);
        }
        if (f->ihsFlag) {
            tryHold(f);
        }

        f->irsAmount = FST_ROT_NONE;
        f->ihsFlag = false;

        // Check lockout (irs/ihs has been applied already)
        if (isCollision(f, f->x, f->y, f->theta)) {
            f->state = FSS_GAMEOVER;
            goto beginTick;
        }

        updateHardDropY(f);
        f->state = FSS_FALLING;
        break;

      case FSS_FALLING:
      case FSS_LANDED:
        // If a hard drop occurs we want to immediately drop the piece and not
        // apply any other movement. This is far more natural and results in
        // less misdrops than if movement is processed prior.
        //
        // See issue #49 for details.
        if ((i->extra & FST_INPUT_HARD_DROP) ||
                // We must recheck the lock timer state here since we may have
                // moved back to FALLING from LANDED on the last frame and do
                // **not** want to lock in mid-air!
                (f->lockTimer >= TICKS(f->lockDelay) && f->state == FSS_LANDED)) {
            f->state = FSS_LINES;

            // Still need to apply piece gravity before entering FSS_LINES.
            doPieceGravity(f, i->gravity);
            break;
        }

        if (i->extra & FST_INPUT_HOLD) {
            tryHold(f);
        }

        if (i->rotation) {
            if (doRotate(f, i->rotation)) {
                rotated = true;
            }
        }

        // Left movement
        distance = i->movement;
        for (; distance < 0; ++distance) {
            if (!isCollision(f, f->x - 1, f->y, f->theta)) {
                f->x -= 1;
                moved = true;
            }
        }

        // Right movement
        for (; distance > 0; --distance) {
            if (!isCollision(f, f->x + 1, f->y, f->theta)) {
                f->x += 1;
                moved = true;
            }
        }

        if (moved || rotated) {
            if (moved) {
                f->se |= FST_SE_FLAG_MOVE;
            }
            if (rotated) {
                f->se |= FST_SE_FLAG_ROTATE;
            }

            updateHardDropY(f);
        }

        doPieceGravity(f, i->gravity);

        // This must occur after we process the lockTimer to allow floorkick
        // limits to be processed correctly. If we encounter a floorkick limit
        // we set the lockTimer to max to allow a lock next frame, while still
        // giving the user an option to perform a move/rotate input.
        if ((moved || rotated) && f->lockStyle == FST_LOCK_MOVE) {
            f->lockTimer = 0;
        }

        if (f->state == FSS_LANDED) {
            f->lockTimer++;
        }

        break;

      case FSS_LINES:
        lockPiece(f);

        // NOTE: Make this conversion less *magic*
        f->se |= (1 << (FST_SE_IPIECE + f->piece));
        f->piece = FS_NONE;

        const int lines = clearLines(f);
        if (0 < lines && lines <= 4) {
            // NOTE: Make this conversion less *magic*
            f->se |= (FST_SE_FLAG_ERASE1 << (lines - 1));
        }

        f->linesCleared += lines;
        f->state = f->linesCleared < f->goal ? FSS_ARE : FSS_GAMEOVER;
        goto beginTick;

      case FSS_GAMEOVER:
        f->se |= FST_SE_FLAG_GAMEOVER;
        // Only save a hiscore if we completed the game (no quit/restart) and
        // if this is not a replay.
        if (!f->replay) {
            fsHiscoreInsert(f);
        }
        /* FALLTHROUGH */

      case FSS_QUIT:
      case FSS_RESTART:
        break;

      default:
        fsLogError("Unknown state entered!");
        break;
    }

    f->totalTicks += 1;
}

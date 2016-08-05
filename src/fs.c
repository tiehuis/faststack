// fs.c
//
// FastStack engine implementation.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "fs.h"
#include "fsInternal.h"

// These map to SRS rotation by standard.
// Different offsets can be applied in fsTables on a case-by-case
// basis.
// Incrementing theta mod FS_NPT will perform consecutive clockwise
// rotations.
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

// What values are stored in each cell on the field
const FSInt pieceColors[7] = {
    0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
};

void fsGameClear(FSGame *f)
{
    // Do this on every new game restart for now
    srand(time(NULL));

    // Most values just need to be zeroed, with some specific
    // values set explicitly for safety.
    memset(f, 0, sizeof(FSGame));

    // Set some default options
    f->lastRandomizer = FSRAND_UNDEFINED;
    f->randomizer = FSRAND_NOSZO_BAG7;

    f->state = FSS_NEW_PIECE;
    f->msPerTick = 16;
    f->rotationSystem = FSROT_SRS;
    f->fieldWidth = 10;
    f->fieldHeight = 20;
    f->goal = 40;
    f->areDelay = 0;
    f->lockDelay = 200;
    f->softDropGravity = 1.25; // 20G gravity
    f->gravity = 0.000625f; // 0.01G (0.01/60)
    f->holdAvailable = true;
    f->holdPiece = FS_NONE;
    f->finesse = 0;
    f->lockStyle = FSLOCK_ENTRY;
    memset(f->b, 0, sizeof(f->b));

    // Initialize the next queue and current
    f->piece = fsNextRandomPiece(f);
    for (int i = 0; i < FS_PREVIEW_MAX; ++i) {
        f->nextPiece[i] = fsNextRandomPiece(f);
    }
}

// Extend to 3-tuple to store colour as well
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

static bool isOccupied(const FSGame *f, int x, int y)
{
    if (x < 0 || x >= f->fieldWidth || y < 0 || y >= f->fieldHeight)
        return true;

    return f->b[y][x] > 1;
}

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

/**
 * Generate a new piece and spawn onto the field.
 */
static void newPiece(FSGame *f)
{
    f->x = f->fieldWidth / 2 - 1;
    f->y = 0;
    f->actualY = 0;
    f->theta = 0;
    f->lockTimer = 0;
    f->finessePieceRotation = 0;
    f->finessePieceDirection = 0;

    // Move all buffered pieces to the next position in queue
    f->piece = f->nextPiece[0];
    memcpy(f->nextPiece, f->nextPiece + 1, FS_PREVIEW_MAX - 1);
    f->nextPiece[FS_PREVIEW_MAX - 1] = fsNextRandomPiece(f);
    f->holdAvailable = true;
}

/**
 * Try to rotate the current piece in the specified direction.
 */
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

        /*
        // Handle this special case
        if (kickData.z == WK_ARIKA_LJT &&
                (f->piece == FS_L || f->piece == FS_J || f->piece == FS_T) {
            switch (f->piece) {
              // Cannot have a block immediately above
              case FS_T:
                if (isOccupied(f, f->x + 1, f->y - 1))
                    return false;
                break;
              case FS_J:
                if (f->theta == 0) {
                }

            }
        }
        */

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

/**
 * Apply the specified gravity to the current piece.
 */
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

// Find all full rows and clear them, moving upper rows down
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

/// The main logic loop of a game.
void fsGameDoTick(FSGame *f, const FSInput *i)
{
    FSInt distance;
    bool moved = false;

    // Store the input encountered so we can debug it on the
    // frontend if required.
    f->lastInput = *i;

beginTick:
    switch (f->state) {
      case FSS_ARE:
          if (f->areTimer++ > TICKS(f->areDelay)) {
              f->areTimer = 0;
              f->state = FSS_NEW_PIECE;
              goto beginTick;
          }
          break;

      case FSS_NEW_PIECE:
        newPiece(f);

        // Check for lockout on spawn
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
        if ((i->extra & FSI_HOLD) && f->holdAvailable) {
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

                updateHardDropY(f);
            }
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
                moved = true;
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

        if (moved) {
            updateHardDropY(f);

            if (f->lockStyle == FSLOCK_MOVE)
                f->lockTimer = 0;
        }

        // Reset lock timer depending on the style

        // Next frame we clear lines if we landed with this piece
        doPieceGravity(f, i->gravity);

        // Check if we are now in a landed state. If a hard drop action was
        // input then bypass this and go directly to line clear.
        if ((i->extra & FSI_HARD_DROP) || f->lockTimer > TICKS(f->lockDelay))
            f->state = FSS_LINES;

        if (f->state == FSS_LANDED)
            f->lockTimer++;

        break;

      case FSS_LINES:
        // Clear the lines in 0 frames (instant) currently
        lockPiece(f);
        f->piece = FS_NONE; // Invalidate piece so it is not drawn
        f->linesCleared += clearLines(f);
        f->state = f->linesCleared < f->goal ? FSS_ARE : FSS_GAMEOVER;
        goto beginTick;

      case FSS_GAMEOVER:
      case FSS_QUIT:
      default:
        break;
    }

    f->totalTicks += 1;
}

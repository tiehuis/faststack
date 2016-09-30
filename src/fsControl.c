///
// fsControl.c
// ===========
//
// Implementation of the function to convert key input into simplified actions
// for the FastStack engine.
///

#include "fs.h"
#include "fsControl.h"
#include "fsInternal.h"

#include <stdio.h>

///
// Transform the current input state into a simple set of actions for the
// engine to apply.
//
// `keys` is an integer with bits set depending on the state of the specified
//  key. The bits set correspond to the `FST_VK_FLAG` enum in `fsControl.h`.
///
void fsVirtualKeysToInput(struct FSInput *dst, FSBits keys, const FSGame *f, FSControl *c)
{
    FSBits lastTickKeys = c->lastKeys;
    FSBits newKeys = keys & ~lastTickKeys;
    c->lastKeys = keys;

    c->currentKeys &= keys;
    keys &= ~c->currentKeys;
    newKeys &= keys;
    dst->currentKeys = keys;

    if (keys & FST_VK_FLAG_LEFT) {
        if (c->dasCounter > TICKS(-f->dasDelay)) {
            if (c->dasCounter >= 0) {
                c->dasCounter = -1;
                dst->movement = -1;
            }
            else {
                c->dasCounter -= 1;
            }
        }
        else {
            int dasSpeed = f->dasSpeed;
            if (dasSpeed) {
                dst->movement = -1;
                c->dasCounter += dasSpeed - 1;
            }
            else {
                dst->movement = -f->fieldWidth;
            }
        }
    }
    else if (keys & FST_VK_FLAG_RIGHT) {
        if (c->dasCounter < TICKS(f->dasDelay)) {
            if (c->dasCounter <= 0) {
                c->dasCounter = 1;
                dst->movement = 1;
            }
            else {
                c->dasCounter += 1;
            }
        }
        else {
            int dasSpeed = f->dasSpeed;
            if (dasSpeed) {
                dst->movement = 1;
                c->dasCounter -= dasSpeed - 1;
            }
            else {
                dst->movement = f->fieldWidth;
            }
        }
    }
    else {
        c->dasCounter = 0;
    }

    const int sdKeysToCheck = f->oneShotSoftDrop ? newKeys : keys;
    if (sdKeysToCheck & FST_VK_FLAG_DOWN) {
        dst->gravity = f->msPerTick * f->softDropGravity;
    }

    if (newKeys & FST_VK_FLAG_ROTL) {
        dst->rotation -= 1;
        dst->extra |= FST_INPUT_FINESSE_ROTATION;
    }
    if (newKeys & FST_VK_FLAG_ROTR) {
        dst->rotation += 1;
        dst->extra |= FST_INPUT_FINESSE_ROTATION;
    }
    // A 180 degree rotation takes priority over any 90 degree rotations
    if (newKeys & FST_VK_FLAG_ROTH) {
        dst->rotation = 2;
    }
    if (newKeys & FST_VK_FLAG_HOLD) {
        dst->extra |= FST_INPUT_HOLD;
    }
    if (newKeys & FST_VK_FLAG_UP) {
        dst->gravity = f->fieldHeight;
        dst->extra |= FST_INPUT_HARD_DROP;
        dst->extra |= FST_INPUT_LOCK;
    }
    if (newKeys & FST_VK_FLAG_LEFT) {
        dst->extra |= FST_INPUT_FINESSE_DIRECTION;
    }
    if (newKeys & FST_VK_FLAG_RIGHT) {
        dst->extra |= FST_INPUT_FINESSE_DIRECTION;
    }
    if (newKeys & FST_VK_FLAG_RESTART) {
        dst->extra |= FST_INPUT_RESTART;
    }
    if (newKeys & FST_VK_FLAG_QUIT) {
        dst->extra |= FST_INPUT_QUIT;
    }
}

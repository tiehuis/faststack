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

///
// Transform the current input state into a simple set of actions for the
// engine to apply.
//
// `keys` is an integer with bits set depending on the state of the specified
//  key. The bits set correspond to the `VKEY` enum in `fsControl.h`.
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

    if (keys & VKEY_LEFT) {
        if (c->dasCounter > TICKS(-c->dasDelay)) {
            if (c->dasCounter >= 0) {
                c->dasCounter = -1;
                dst->movement = -1;
            }
            else {
                c->dasCounter -= 1;
            }
        }
        else {
            int dasSpeed = c->dasSpeed;
            if (dasSpeed) {
                dst->movement = -1;
                c->dasCounter += dasSpeed - 1;
            }
            else {
                dst->movement = -f->fieldWidth;
            }
        }
    }
    else if (keys & VKEY_RIGHT) {
        if (c->dasCounter < TICKS(c->dasDelay)) {
            if (c->dasCounter <= 0) {
                c->dasCounter = 1;
                dst->movement = 1;
            }
            else {
                c->dasCounter += 1;
            }
        }
        else {
            int dasSpeed = c->dasSpeed;
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
    if (sdKeysToCheck & VKEY_DOWN) {
        dst->gravity = f->msPerTick * f->softDropGravity;
    }

    if (newKeys & VKEY_ROTL) {
        dst->rotation -= 1;
        dst->extra |= FSI_FINESSE_ROTATION;
    }
    if (newKeys & VKEY_ROTR) {
        dst->rotation += 1;
        dst->extra |= FSI_FINESSE_ROTATION;
    }
    // A 180 degree rotation takes priority over any 90 degree rotations
    if (newKeys & VKEY_ROTH) {
        dst->rotation = 2;
    }
    if (newKeys & VKEY_HOLD) {
        dst->extra |= FSI_HOLD;
    }
    if (newKeys & VKEY_UP) {
        dst->gravity = f->fieldHeight;
        dst->extra |= FSI_HARD_DROP;
        dst->extra |= FSI_LOCK;
    }
    if (newKeys & VKEY_LEFT) {
        dst->extra |= FSI_FINESSE_DIRECTION;
    }
    if (newKeys & VKEY_RIGHT) {
        dst->extra |= FSI_FINESSE_DIRECTION;
    }
}

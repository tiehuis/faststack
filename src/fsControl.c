// fsControl.c
//
// Bridge between virtual keys -> game input.

#include "fs.h"
#include "fsControl.h"
#include "fsInternal.h"

// Map a set of virtual key presses to the current tick input using the current
// control state.
void fsVirtualKeysToInput(struct FSInput *dst, FSBits keys, const FSGame *f, FSControl *c)
{
    FSBits lastTickKeys = c->lastKeys;
    FSBits newKeys = keys & ~lastTickKeys;
    c->lastKeys = keys;

    FSBits releasedKeys = ~keys & lastTickKeys;
    releasedKeys &= ~c->currentKeys;
    c->currentKeys &= keys;
    keys &= ~c->currentKeys;

    newKeys &= keys;

    // Handle DAS
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

    if (keys & VKEY_DOWN) {
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
    // Override any single lr rotation
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

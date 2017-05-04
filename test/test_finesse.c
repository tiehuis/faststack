#include "framework.h"

// Input
#if 0
    i8 rotation;

    /// A left-right movement action.
    //
    // Positive movement indicates a right move, whilst negative is left.
    i8 movement;

    /// Downward movement action. Product of gravity and soft drop.
    i8 gravity;

    /// Specific extra movement (e.g. HardDrop).
    i8 extra;

    /// How many new keys were pressed (used for finesse/KPT)
    i8 newKeysCount;

    /// Current key status (used for some specific events)
    u32 currentKeys;
#endif

// Finesse is currently only applicable for standard 10-width playfields.
void initFinesseTest(FSEngine *f)
{
    fsGameInit(f);
    f->fieldWidth = 10;
}

// Reset the playfield with a new piece at the specified location.
void resetFinesseTest(FSEngine *f, int pieceType)
{
    fsGameReset(f);
    f->state = FSS_FALLING;
    f->piece = pieceType;

    f->x = f->fieldWidth / 2 - 2;
    f->y = 1;
    f->actualY = fix(f->y);
    f->theta = 0;
}

static void iTest(void)
{
    FSEngine f;
    FSInput in;

    resetFinesseTest(&f, FS_I);

    memset(&in, 0, sizeof(in));
    in.movement = -10;
    fsGameTick(&f, &in);

    memset(&in, 0, sizeof(in));
    in.extra = FST_INPUT_HARD_DROP;
    fsGameTick(&f, &in);

    memset(&in, 0, sizeof(in));
    fsGameTick(&f, &in);

    printf("I: %d - %d\n", -10, f.finesse);
}

int main(void)
{
    FSEngine f;
    initFinesseTest(&f);

    iTest();
}

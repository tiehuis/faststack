///
// finesse.c
// =========
//
// Calculates the finesse needed for a given piece.
//
// The tables and algorithm are adapted from belzebub's NullpoMino mod.
//
// Note: Finesse faults can only be calculated for standard play widths
// of 10.
///

#include "core.h"

// Finesse can only be calculated if the field is a standard width of 10.
//
// Note: We should calculate these tables from a formula instead to save on
// memory.
static const i8 rotation[FS_NPT][FS_NPR][10] = {
    [FS_I] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    },
    [FS_J] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    },
    [FS_L] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    },
    [FS_O] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    [FS_S] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    },
    [FS_T] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    },
    [FS_Z] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    }
};

static const i8 movement[FS_NPT][FS_NPR][10] = {
    [FS_I] = {
        {1, 2, 1, 0, 1, 2, 1, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
        {1, 2, 1, 0, 1, 2, 1, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 1, 1, 1, 1}
    },
    [FS_J] = {
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 1, 2, 1, 0, 1, 2, 2, 1, 0},
        {1, 2, 1, 0, 1, 2, 1, 0, 0, 0},
        {1, 2, 1, 0, 1, 2, 2, 1, 1, 0}
    },
    [FS_L] = {
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 1, 2, 1, 0, 1, 2, 2, 1, 0},
        {1, 2, 1, 0, 1, 2, 1, 0, 0, 0},
        {1, 2, 1, 0, 1, 2, 2, 1, 1, 0}
    },
    [FS_O] = {
        {1, 2, 2, 1, 0, 1, 2, 2, 1, 0},
        {1, 2, 2, 1, 0, 1, 2, 2, 1, 0},
        {1, 2, 2, 1, 0, 1, 2, 2, 1, 0},
        {1, 2, 2, 1, 0, 1, 2, 2, 1, 0},
    },
    [FS_S] = {
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 1, 1, 0, 0, 1, 2, 1, 1, 0},
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 1, 1, 0, 0, 1, 2, 1, 1, 0}
    },
    [FS_T] = {
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 1, 2, 1, 0, 1, 2, 2, 1, 0},
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 2, 1, 0, 1, 2, 2, 1, 1, 0}
    },
    [FS_Z] = {
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 1, 1, 0, 0, 1, 2, 1, 1, 0},
        {1, 2, 1, 0, 1, 2, 2, 1, 0, 0},
        {1, 1, 1, 0, 0, 1, 2, 1, 1, 0}
    }
};

static const i8 positionDelta[FS_NPT][FS_NPR] = {
    [FS_I] = {0, 2, 0, 1},
    [FS_J] = {0, 1, 0, 0},
    [FS_L] = {0, 1, 0, 0},
    [FS_O] = {0, 2, 0, 1},
    [FS_S] = {0, 1, 0, 0},
    [FS_T] = {0, 1, 0, 0},
    [FS_Z] = {0, 1, 0, 0}
};

// Compute the minimum number of rotations and movements to get to the
// specified location.
//
// It is up to the caller to ensure that the field width is exactly 10.
//
// Note: Finesse does not take into account overhangs and the like. We also
// are only interested in rows since it doesn't matter the y value when
// performing a hard drop.
i8x2 fsMinimalFinesseCount(FSBlock piece, i8 x, i8 theta)
{
    const i8 delta = positionDelta[piece][theta];
    const i8x2 finesse = {
        .x = rotation[piece][theta][delta + x],
        .y = movement[piece][theta][delta + x]
    };

    return finesse;
}

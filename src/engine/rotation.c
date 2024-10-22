///
// rotation.c
// ==========
//
// A rotation system consists of wallkicks and initial piece offsets.
//
// We index the kick tables using the followwing ordering:
//  * I, J, L, O, S, T, Z
//
// All tables are computed from their beginning direction. To compute the
// wallkick for the rotation `0 -> R` we would check table.kicksR[0];
///

#include "rotation.h"
#include "internal.h"

// The C99 standard specifies that static variables are 0 initialized
// (section 6.7.8 item 10) which we rely on during initialization.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// Shorthand for specifying the end of a wallkick table.
#define WKE {.z = WK_END}

// Shorthand for specifying an empty kick table.
#define NO_KICKS -1, -1, -1, -1, -1, -1, -1

///
// Static piece offsets.
//
// These map to SRS rotation by default. Alternate rotation systems
// as specified by adjusting their wallkick tables to suit.
//
// This complicates wallkicks for some otherwise simple rotations, but
// in my experience is cleaner than implementing different base offsets.
const i8x2 pieceOffsets[FS_NPT][FS_NPR][FS_NBP] = {
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

const WallkickTable emptyWallkickTable = {
    {{0, 0}, WKE}, // R -> U
    {{0, 0}, WKE}, // D -> R
    {{0, 0}, WKE}, // L -> D
    {{0, 0}, WKE}  // U -> L
};

///
// Simple
//
// Performs no wallkicks.
///
static const FSRotationSystem rotSimple = {
    .kicksR = {NO_KICKS},
    .kicksL = {NO_KICKS},
    .kicksH = {NO_KICKS}
};

///
// SRS
//
// Performs wallkicks adhering to the Super Rotation System.
///
static const FSRotationSystem rotSRS = {
    .kicksR = {1, 0, 0, -1, 0, 0, 0},
    .kicksL = {3, 2, 2, -1, 2, 2, 2},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: JLSTZ clockwise
        {
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 0 -> R
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 2 -> L
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 0
        },

        // 1: I clockwise
        {
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}, WKE}, // 0 -> R
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}, WKE}, // R -> 2
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}, WKE}  // L -> 0
        },

        // 2: JLSTZ anticlockwise
        {
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 0 -> L
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 2
        },

        // 3: I anticlockwise
        {
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}, WKE}, // 0 -> L
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}, WKE}, // R -> 0
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}, WKE}, // 2 -> R
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}, WKE}  // L -> 2
        }
    }
};

///
// Arika SRS
//
// Similar to SRS with a different set of I wallkicks.
///
static const FSRotationSystem rotArikaSRS = {
    .kicksR = {1, 0, 0, -1, 0, 0, 0},
    .kicksL = {3, 2, 2, -1, 2, 2, 2},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: JLSTZ clockwise
        {
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 0 -> R
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 2 -> L
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 0
        },

        // 1: I clockwise
        {
            {{ 0, 0}, {-2, 0}, { 1, 0}, { 1,-2}, {-2, 1}, WKE}, // 0 -> R
            {{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}, WKE}, // R -> 2
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 1}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1,-2}, {-2, 1}, WKE}  // L -> 0
        },

        // 2: JLSTZ anticlockwise
        {
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}, WKE}, // 0 -> L
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}, WKE}  // L -> 2
        },

        // 3: I anticlockwise
        {
            {{ 0, 0}, { 2, 0}, {-1, 0}, {-1,-2}, { 2, 1}, WKE}, // 0 -> L
            {{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}, WKE}, // R -> 0
            {{ 0, 0}, {-2, 0}, { 1, 0}, {-2,-1}, { 1, 1}, WKE}, // 2 -> R
            {{ 0, 0}, { 1, 0}, {-2, 0}, { 1,-2}, {-2, 2}, WKE}  // L -> 2
        }
    }
};

///
// Sega
//
// Sega rotation system. This has no wallkicks, the only complication is
// mapping the SRS internal rotation system to the sega one.
///
static const FSRotationSystem rotSega = {
    .entryTheta = {0, 2, 2, 0, 0, 2, 0},
    .kicksR = {0, 2, 2,-1, 4, 2, 6},
    .kicksL = {1, 3, 3,-1, 5, 3, 7},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: I clockwise
        {
            {{ 0, 0}, WKE},  // 0 -> R
            {{ 0,-1}, WKE},  // R -> 2
            {{ 1, 1}, WKE},  // 2 -> L
            {{-1, 0}, WKE}   // L -> 0
        },

        // 1: I anticlockwise
        {
            {{ 1, 0}, WKE},  // 0 -> L
            {{ 0, 0}, WKE},  // R -> 0
            {{ 0, 1}, WKE},  // 2 -> R
            {{-1,-1}, WKE}   // L -> 2
        },

        // 2: JLT clockwise
        {
            {{ 0, 0}, WKE},  // 0 -> R
            {{ 0, 1}, WKE},  // R -> 2
            {{ 0,-1}, WKE},  // 2 -> L
            {{ 0, 0}, WKE},  // L -> 0
        },

        // 3: JLT anticlockwise
        {
            {{ 0, 0}, WKE},  // 0 -> L
            {{ 0, 0}, WKE},  // R -> 0
            {{ 0,-1}, WKE},  // 2 -> R
            {{ 0, 1}, WKE},  // L -> 2
        },

        // 4: S clockwise
        {   {{-1,-1}, WKE},  // 0 -> R
            {{ 1, 0}, WKE},  // R -> 2
            {{ 0, 0}, WKE},  // 2 -> L
            {{ 0, 1}, WKE}   // L -> 0
        },

        // 5: S anticlockwise
        {
            {{ 0,-1}, WKE},  // 0 -> L
            {{ 1, 1}, WKE},  // R -> 0
            {{-1, 0}, WKE},  // 2 -> R
            {{ 0, 0}, WKE}   // L -> 2
        },

        // 6: Z clockwise
        {
            {{ 0,-1}, WKE},  // 0 -> R
            {{ 0, 0}, WKE},  // R -> 2
            {{ 1, 0}, WKE},  // 2 -> L
            {{-1, 1}, WKE}   // L -> 0
        },

        // 7: Z anticlockwise
        {
            {{ 1,-1}, WKE},  // 0 -> L
            {{ 0, 1}, WKE},  // R -> 0
            {{ 0, 0}, WKE},  // 2 -> R
            {{-1, 0}, WKE}   // L -> 2
        },
    }
};

///
// TGM1/2
//
// An extension of the Sega rotation system.
///
static const FSRotationSystem rotTGM12 = {
    .entryTheta = {0, 2, 2, 0, 0, 2, 0},
    .kicksR = {0, 2, 2,-1, 4, 2, 6},
    .kicksL = {1, 3, 3,-1, 5, 3, 7},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: I clockwise
        {
            {{ 0, 0}, WKE},  // 0 -> R
            {{ 0,-1}, WKE},  // R -> 2
            {{ 1, 1}, WKE},  // 2 -> L
            {{-1, 0}, WKE}   // L -> 0
        },

        // 1: I anticlockwise
        {
            {{ 1, 0}, WKE},  // 0 -> L
            {{ 0, 0}, WKE},  // R -> 0
            {{ 0, 1}, WKE},  // 2 -> R
            {{-1,-1}, WKE}   // L -> 2
        },

        // 2: JLT clockwise
        {
            {{ 0, 0}, {.z = WK_ARIKA_LJT}, { 1, 0}, {-1, 0}, WKE},  // 0 -> R
            {{ 0, 1},                      { 1, 1}, {-1, 1}, WKE},  // R -> 2
            {{ 0,-1}, {.z = WK_ARIKA_LJT}, { 1,-1}, {-1,-1}, WKE},  // 2 -> L
            {{ 0, 0},                      { 1, 0}, {-1, 0}, WKE},  // L -> 0
        },

        // 3: JLT anticlockwise
        {
            {{ 0, 0}, {.z = WK_ARIKA_LJT}, { 1, 0}, {-1, 0}, WKE},  // 0 -> L
            {{ 0, 0},                      { 1, 0}, {-1, 0}, WKE},  // R -> 0
            {{ 0,-1}, {.z = WK_ARIKA_LJT}, { 1,-1}, {-1,-1}, WKE},  // 2 -> R
            {{ 0, 1},                      { 1, 1}, {-1, 1}, WKE},  // L -> 2
        },

        // 4: S clockwise
        {   {{-1,-1}, { 0,-1}, {-2,-1}, WKE},  // 0 -> R
            {{ 1, 0}, { 2, 0}, { 0, 0}, WKE},  // R -> 2
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE},  // 2 -> L
            {{ 0, 1}, { 1, 1}, {-1, 1}, WKE}   // L -> 0
        },

        // 5: S anticlockwise
        {
            {{ 0,-1}, { 1,-1}, {-1,-1}, WKE},  // 0 -> L
            {{ 1, 1}, { 2, 1}, { 0, 1}, WKE},  // R -> 0
            {{-1, 0}, { 0, 0}, { 2, 0}, WKE},  // 2 -> R
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE}   // L -> 2
        },

        // 6: Z clockwise
        {
            {{ 0,-1}, { 1,-1}, {-1,-1}, WKE},  // 0 -> R
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE},  // R -> 2
            {{ 1, 0}, { 2, 0}, { 0, 0}, WKE},  // 2 -> L
            {{-1, 1}, { 0, 1}, {-2,-1}, WKE}   // L -> 0
        },

        // 7: Z anticlockwise
        {
            {{ 1,-1}, { 2,-1}, { 0,-1}, WKE},  // 0 -> L
            {{ 0, 1}, { 1, 1}, {-1, 1}, WKE},  // R -> 0
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE},  // 2 -> R
            {{-1, 0}, { 0, 0}, {-2, 0}, WKE}   // L -> 2
        }
    }
};

///
// TGM3
//
// An extension to the Sega rotation system.
//
// This is the same as TGM1/2 with the addition of
//  * I wallkicks
//  * T, I floorkicks
//
// Notes:
//  * Current implementation does not handle two-shot floorkicks. This needs to
//    be done in the actual engine.
//
//    ADD CORRECT LR I WALLKICK
///
static const FSRotationSystem rotTGM3 = {
    .entryTheta = {0, 2, 2, 0, 0, 2, 0},
    .kicksR = {0, 2, 2,-1, 4, 8, 6},
    .kicksL = {1, 3, 3,-1, 5, 9, 7},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: I clockwise
        {
            {{ 0, 0}, { 0,-1}, { 0,-2},          WKE},  // 0 -> R
            {{ 0,-1}, { 1,-1}, { 2,-1}, {-1,-1}, WKE},  // R -> 2
            {{ 1, 1}, { 1, 0}, { 1,-1},          WKE},  // 2 -> L
            {{-1, 0}, { 0, 0}, { 1, 0}, {-2, 0}, WKE}   // L -> 0
        },

        // 1: I anticlockwise
        {
            {{ 1, 0}, { 1,-1}, { 1,-2},          WKE},  // 0 -> L
            {{ 0, 0}, { 1, 0}, { 2, 0}, {-1, 0}, WKE},  // R -> 0
            {{ 0, 1}, { 0, 0}, { 0,-1},          WKE},  // 2 -> R
            {{-1,-1}, { 0,-1}, { 1,-1}, {-2,-1}, WKE}   // L -> 2
        },

        // 2: JL clockwise
        {
            {{ 0, 0}, {.z = WK_ARIKA_LJT}, { 1, 0}, {-1, 0}, WKE},  // 0 -> R
            {{ 0, 1},                      { 1, 1}, {-1, 1}, WKE},  // R -> 2
            {{ 0,-1}, {.z = WK_ARIKA_LJT}, { 1,-1}, {-1,-1}, WKE},  // 2 -> L
            {{ 0, 0},                      { 1, 0}, {-1, 0}, WKE},  // L -> 0
        },

        // 3: JL anticlockwise
        {
            {{ 0, 0}, {.z = WK_ARIKA_LJT}, { 1, 0}, {-1, 0}, WKE},  // 0 -> L
            {{ 0, 0},                      { 1, 0}, {-1, 0}, WKE},  // R -> 0
            {{ 0,-1}, {.z = WK_ARIKA_LJT}, { 1,-1}, {-1,-1}, WKE},  // 2 -> R
            {{ 0, 1},                      { 1, 1}, {-1, 1}, WKE},  // L -> 2
        },

        // 4: S clockwise
        {   {{-1,-1}, { 0,-1}, {-2,-1}, WKE},  // 0 -> R
            {{ 1, 0}, { 2, 0}, { 0, 0}, WKE},  // R -> 2
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE},  // 2 -> L
            {{ 0, 1}, { 1, 1}, {-1, 1}, WKE}   // L -> 0
        },

        // 5: S anticlockwise
        {
            {{ 0,-1}, { 1,-1}, {-1,-1}, WKE},  // 0 -> L
            {{ 1, 1}, { 2, 1}, { 0, 1}, WKE},  // R -> 0
            {{-1, 0}, { 0, 0}, { 2, 0}, WKE},  // 2 -> R
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE}   // L -> 2
        },

        // 6: Z clockwise
        {
            {{ 0,-1}, { 1,-1}, {-1,-1}, WKE},  // 0 -> R
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE},  // R -> 2
            {{ 1, 0}, { 2, 0}, { 0, 0}, WKE},  // 2 -> L
            {{-1, 1}, { 0, 1}, {-2,-1}, WKE}   // L -> 0
        },

        // 7: Z anticlockwise
        {
            {{ 1,-1}, { 2,-1}, { 0,-1}, WKE},  // 0 -> L
            {{ 0, 1}, { 1, 1}, {-1, 1}, WKE},  // R -> 0
            {{ 0, 0}, { 1, 0}, {-1, 0}, WKE},  // 2 -> R
            {{-1, 0}, { 0, 0}, {-2, 0}, WKE}   // L -> 2
        },

        // 8: T clockwise
        {
            {{ 0, 0}, {.z = WK_ARIKA_LJT}, { 1, 0}, {-1, 0},         WKE},  // 0 -> R
            {{ 0, 1},                      { 1, 1}, {-1, 1}, {0, 0}, WKE},  // R -> 2
            {{ 0,-1}, {.z = WK_ARIKA_LJT}, { 1,-1}, {-1,-1},         WKE},  // 2 -> L
            {{ 0, 0},                      { 1, 0}, {-1, 0}, {0,-1}, WKE},  // L -> 0
        },

        // 9: T anticlockwise
        {
            {{ 0, 0}, {.z = WK_ARIKA_LJT}, { 1, 0}, {-1, 0},         WKE},  // 0 -> L
            {{ 0, 0},                      { 1, 0}, {-1, 0}, {0,-1}, WKE},  // R -> 0
            {{ 0,-1}, {.z = WK_ARIKA_LJT}, { 1,-1}, {-1,-1},         WKE},  // 2 -> R
            {{ 0, 1},                      { 1, 1}, {-1, 1}, {0, 0}, WKE},  // L -> 2
        },
    }
};

///
// DTET
//
// A symmetric system from the DTET series of games. Can be considered a
// simplification of the TGM1/2 rotation system.
///
static const FSRotationSystem rotDTET = {
    .entryTheta = {0, 2, 2, 0, 0, 2, 0},
    .kicksR = {0, 0, 0, -1, 0, 0, 0},
    .kicksL = {1, 1, 1, -1, 1, 1, 1},
    .kicksH = {NO_KICKS},
    .kickTables = {
        // 0: Clockwise
        {
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}, // O -> R
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}, // R -> 2
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}, // 2 -> L
            {{ 0, 0}, { 1, 0}, {-1, 0}, { 0, 1}, { 1, 1}, {-1, 1}, WKE}  // L -> 0
        },

        // 1: Anticlockwise
        {
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}, // 0 -> L
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}, // R -> 0
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}, // 2 -> R
            {{ 0, 0}, {-1, 0}, { 1, 0}, { 0, 1}, {-1, 1}, { 1, 1}, WKE}  // L -> 2
        }
    }
};

///
// List of all available rotation systems.
///
const FSRotationSystem *rotationSystems[FS_NRS] = {
    &rotSimple, &rotSega, &rotSRS, &rotArikaSRS, &rotTGM12, &rotTGM3, &rotDTET
};

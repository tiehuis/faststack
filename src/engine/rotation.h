///
// rotation.h
// ==========
//
// Defines structures for working with wallkick/rotation data. This data is
// exposed through public externs and and is accessed through indexing via the
// `FST_ROTSYS_*` enumeration values.
///

#ifndef FS_ROTATION_H
#define FS_ROTATION_H

#include "core.h"
#include "config.h"

enum RotationSystemType {
    FST_ROTSYS_SIMPLE,
    FST_ROTSYS_SEGA,
    FST_ROTSYS_SRS,
    FST_ROTSYS_ARIKA_SRS,
    FST_ROTSYS_TGM12,
    FST_ROTSYS_TGM3,
    FST_ROTSYS_DTET
};

enum RotationAmount {
    FST_ROT_NONE = 0,
    FST_ROT_CLOCKWISE = 1,
    FST_ROT_ANTICLOCKWISE = -1,
    FST_ROT_HALFTURN = 2
};

///
// A wallkick table consists of a number 'tests' which are tested in order
// until success or every test has been tried.
///
typedef i8x3 WallkickTable[FS_NPR][FS_MAX_KICK_LEN];

///
// Specifies a single rotation system.
//
// A rotation system is comprised of three main parts:
//
//  * Entry Offsets
//      Specifies x, y offsets of a piece when it initially spawns.
//
//  * Entry Theta
//      Specifies the rotation state of a piece when it initially spawns.
//
//  * Kick Tables and Kick Indexes
//      Specifies individual wallkick tables for a given piece. Tables can
//      be shared amongst types by reusing the index.
///
struct FSRotationSystem {
    /// Initial x, y offsets.
    i8 entryOffset[FS_NPT];

    /// Initial theta offets.
    i8 entryTheta[FS_NPT];

    /// Indexes into 'kickTables'.
    i8 kicksL[FS_NPT];
    i8 kicksR[FS_NPT];
    i8 kicksH[FS_NPT];

    /// A sequence of wallkick tests.
    WallkickTable kickTables[FS_MAX_NO_OF_WALLKICK_TABLES];
};

///
// The core piece offsets used.
///
extern const i8x2 pieceOffsets[FS_NPT][FS_NPR][FS_NBP];

///
// Rotation Systems are defined statically. We only store an index to the
// currently used table in 'FSEngine'.
///
extern const FSRotationSystem *rotationSystems[FS_NRS];

///
// An empty wallkick table.
///
extern const WallkickTable emptyWallkickTable;

#endif // FS_ROTATION_H

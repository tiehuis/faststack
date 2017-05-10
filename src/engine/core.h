///
// core.h
// ======
//
// Core public definitions shared amongst all files.
///

#ifndef FS_TYPES_H
#define FS_TYPES_H

#include "fslibc.h"

// Every structure is typedef here instead of at their declaration. The reason
// for this is to remove the requirement to forward-declare these types
// anywhere else (as a struct, too). This is cleaner, and works much better
// due to all the cyclical dependencies between these structures.
typedef struct FSEngine FSEngine;
typedef struct FSInput FSInput;
typedef struct FSControl FSControl;
typedef struct FSView FSView;
typedef struct FSFrontend FSFrontend;
typedef struct FSOptions FSOptions;
typedef struct FSReplay FSReplay;
typedef struct FSRotationSystem FSRotationSystem;
typedef struct FSRandCtx FSRandCtx;

// (N)umber of (P)iece (T)ypes.
#define FS_NPT 7

// (N)umber of (R)otation (S)ystems.
#define FS_NRS 7

// (N)umber of (P)iece (R)otations.
#define FS_NPR 4

// (N)umber of (B)locks per (P)iece.
#define FS_NBP 4

// Convert an arbitary index type into its corresponding flag type.
//
// e.g. FST_SE_* -> FST_SE_FLAG_*
#define FS_TO_FLAG(x) (1 << (x))

enum PieceType {
    FS_I,
    FS_J,
    FS_L,
    FS_O,
    FS_S,
    FS_T,
    FS_Z,
    FS_NONE
};

typedef int8_t FSBlock;
typedef int8_t i8;
typedef int32_t i32;
typedef uint32_t u32;

typedef struct i8x2 {
    i8 x;
    i8 y;
} i8x2;

typedef struct i8x3 {
    i8 x;
    i8 y;
    i8 z;
} i8x3;

#endif // FS_TYPES_H

///
// fsCore.h
// =========
//
// Core public definitions across all files.
///

#ifndef FS_TYPES_H
#define FS_TYPES_H

#include <stdint.h>

/// Number of types of pieces.
#define FS_NPT 7

/// Number of rotation systems.
#define FS_NRS 7

/// Number of rotation states.
#define FS_NPR 4

/// Number of blocks in a piece.
#define FS_NBP 4

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

// Represents a single block id
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

///
// Convert an arbitrary index type (e.g. FST_SE_*) into its corresponding flag
// value (FST_SE_FLAG_*).
#define FS_TO_FLAG(x) (1 << (x))

#endif // FS_TYPES_H

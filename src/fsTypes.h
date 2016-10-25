///
// fsTypes.h
// =========
//
// Notes:
//  * Rename this to fsCore.h
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

/// Piece types
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

/// Represents a single piece id.
typedef int8_t FSBlock;

typedef int8_t i8;
typedef int32_t i32;
typedef uint32_t u32;

/// Generic 2-member tuple of type 'i8'.
typedef struct i8x2 {
    i8 x;
    i8 y;
} i8x2;

/// Generic 3-member tuple of type 'i8'.
typedef struct i8x3 {
    i8 x;
    i8 y;
    i8 z;
} i8x3;

#endif // FS_TYPES_H

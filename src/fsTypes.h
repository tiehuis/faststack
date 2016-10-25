///
// fsTypes.h
// =========
//
// Notes:
//  * Consider using types from stdint.h directly where required.
///

#ifndef FS_TYPES_H
#define FS_TYPES_H

#include <stdint.h>

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

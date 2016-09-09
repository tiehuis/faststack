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

/// A type for representing a sequence of bits.
typedef uint_fast32_t FSBits;

/// An integer type for small variables. Not required to exceed 8 bits.
typedef int_fast8_t FSInt;

/// An integer type for large variables. At least 32 bits long.
typedef int_least32_t FSLong;

/// Generic 2-member tuple of type 'FSInt'.
typedef struct FSInt2 {
    FSInt x;
    FSInt y;
} FSInt2;

/// Generic 3-member tuple of type 'FSInt'.
typedef struct FSInt3 {
    FSInt x;
    FSInt y;
    FSInt z;
} FSInt3;

#endif // FS_TYPES_H

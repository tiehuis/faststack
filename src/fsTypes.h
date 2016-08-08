///
// fsTypes.h
//
// Types used internally and externally.
//
// Notes:
//  - Consider using the stdint types directly instead of creating typedefs.
//    The benefit is only if targetting a different platform or micro
//    optimization for space.
///

#ifndef FSTYPES_H
#define FSTYPES_H

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

#endif

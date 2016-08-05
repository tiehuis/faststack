// FastStack Internal type definitions.

#ifndef FSTYPES_H
#define FSTYPES_H

#include <stdint.h>

// A single tetrimino block/id
typedef int8_t FSBlock;

// An integer type for bit sequences
typedef uint_fast32_t FSBits;

// An integer type for small configuration variables
// Not required to exceed 2^8.
typedef int_fast8_t FSInt;

// An integer type for large configuration variables
typedef int_least32_t FSLong;

// A 2-member tuple of type FSInt
typedef struct FSInt2 {
    FSInt x;
    FSInt y;
} FSInt2;

// A 3-member tuple of type FSInt
typedef struct FSInt3 {
    FSInt x;
    FSInt y;
    FSInt z;
} FSInt3;

#endif

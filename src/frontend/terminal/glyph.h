///
// glyph.h
// =======
//
// Defines a number of glyph sets that can be used when displaying field
// items.
//
// These are currently hardcoded as sets, but we should allow the user to
// specify a base set to use, and allow overriding of individual blocks as
// well.
//
// Note: Since unicode is a real pain in C.
///

#ifndef GLYPH_H
#define GLYPH_H

#include <stdint.h>

typedef struct {
    // Block segments
    uint32_t blockL;
    uint32_t blockR;
    uint32_t blockE;

    // Border segments
    uint32_t borderR;
    uint32_t borderL;
    uint32_t borderB;
    uint32_t borderLB;
    uint32_t borderRB;
} GlyphSet;

extern const GlyphSet asciiGlyphSet;
extern const GlyphSet unicodeGlyphSet;

#endif // GLYPH_H

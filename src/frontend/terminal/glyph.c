///
// glyph.c
// =======
//
// Definitions fors glyph sets that can be used.
///

#include "glyph.h"

const GlyphSet asciiGlyphSet = {
    .blockL = ' ',
    .blockR = ' ',
    .blockE = ' ',

    .borderR = '|',
    .borderL = '|',
    .borderB = '_',
    .borderLB = '_',
    .borderRB = '_'
};

const GlyphSet unicodeGlyphSet = {
    .blockL = ' ',
    .blockR = ' ',
    .blockE = ' ',

    .borderR = 0x8294E2,
    .borderL = 0x8294E2,
    .borderB = 0x8094E2,
    .borderLB = 0x9494E2,
    .borderRB = 0x9894E2
};

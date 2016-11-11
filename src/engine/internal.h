///
// internal.h
// ==========
//
// Internal macros and definitions private to engine implementation.
///

#ifndef FS_INTERNAL_H
#define FS_INTERNAL_H

// Sentinel value for terminating a wallkick test array.
#define WK_END 0x71

// Wallkick value for signalling a TGM1/2 rotation condition test.
#define WK_ARIKA_LJT 0x70

// Convert ms into the corresponding ticks value. This assumes that an
// `FSEngine` is within scope and bound to the variable `f`.
#define TICKS(x) ((x) / (f->msPerTick))

#endif // FS_INTERNAL_H

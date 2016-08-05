// fsInternal.h
//
// Internal macros used across implementations.

// Sentinel value indicating the end of a wallkick
#define WK_END 0x71

// Specifies to check for the TGM1,2 rotation condition
#define WK_ARIKA_LJT 0x70

// Convert milliseconds to ticks using the FSGame instance in variable f.
#define TICKS(x) ((x) / (f->msPerTick))

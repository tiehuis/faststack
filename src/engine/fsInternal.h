///
// fsInternal.h
// ============
//
// Internal macros and definitions that are used across multiple files.
//
// This is not included in the public header `fsEngine.h`.
///

#ifndef FS_INTERNAL_H
#define FS_INTERNAL_H

/// Wallkick: Sentinel value indicating no more wallkick tests are left.
#define WK_END 0x71

/// Wallkick: Flag specifying that the TGM1, 2 rotation condition should be
//            tested.
#define WK_ARIKA_LJT 0x70

/// General: Convert milliseconds value to ticks.
#define TICKS(x) ((x) / (f->msPerTick))

#endif // FS_INTERNAL_H

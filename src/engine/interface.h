///
// interface.h
// ===========
//
// Defines the interface which is required to be implemented in its entirety
// by any frontend implementation.
//
// All interface functions are prefixed with `fsi` to differentiate them from
// the standard provided functions (prefixed with `fs`).
// frontend.
//
// Each frontend implementation is expected to implement an actual `FSPSView`
// type itself.
//
// This **should** be declared with one field `view` of type `FSView`.
//
// Notes:
//  * Do we require microsecond granularity for `fsiGetTime` and `fsiSleepUs`?.
//  * Weak-linking potentially unimplemented functions could improve
//    ergonomics. The problem with this is that it requires non-standard
//    extensions.
///

#ifndef FS_INTERFACE_H
#define FS_INTERFACE_H

#include "core.h"
#include "engine.h"


///
// Pre-initialize. This is currently necessary as the structure needs to
// be partially initialized to allow proper loading of read structures.
//
// Notes:
//  * This does not load the graphics themselves and should be removed in an
//    ideal world.
///
void fsiPreInit(FSPSView *v);

///
// Initialize the FSPSView structure.
///
void fsiInit(FSPSView *v);

///
// Free any data of a FSPSView structure.
///
void fsiFree(FSPSView *v);

///
// TODO:
// This would be less efficient than readKeys but can be used by the menu-like
// keys which do not support rebinding themselves.
// int fsiKeyIsPressed(FSPSView *v);

///
// Render the specified string in the center of the field.
//
// This is used for strings such as:
//
//  * "READY"
//  * "GO"
//  * "EXCELLENT"
///
void fsiRenderFieldString(FSPSView *v, const char *msg);

///
// Return the current time with microsecond granularity.
//
// The reference clock should be monotonic.
///
i32 fsiGetTime(FSPSView *v);

///
// Sleep for the specific number of microseconds.
///
void fsiSleepUs(FSPSView *v, i32 time);

///
// Return the set of virtual keys that are currently pressed.
//
// The translation from physical keys to virtual keys must be handled by the
// frontend.
///
u32 fsiReadKeys(FSPSView *v);

///
// Draw the specified view to the screen.
///
void fsiDraw(FSPSView *v);

///
// Blit any pending screen changes to the screen.
///
void fsiBlit(FSPSView *v);

///
// This hook is called at the start of every frame.
///
void fsiPreFrameHook(FSPSView *v);

///
// This hook is called at the end of every frame (before we sleep).
///
void fsiPostFrameHook(FSPSView *v);

///
// Play the specified sound effect.
///
void fsiPlaySe(FSPSView *v, u32 se);

///
// Try to register the specified key with the views keymap.
///
void fsiAddToKeymap(FSPSView *v, const int vkey, const char *key, bool isDefault);

///
// Process an key-value pair option.
///
void fsiUnpackFrontendOption(FSPSView *v, const char *key, const char *value);

#endif // FS_INTERFACE_H

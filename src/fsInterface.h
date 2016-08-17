///
// fsInterface.h
//
// The main interface between frontend and engine code.
//
// All functions prefixed with fsi* **must** be implemented by a
// frontend.
//
// Notes:
//  - Do we require microsecond granularity for `fsiGetTime` and `fsiSleepUs`?.
//    Millisecond is probably sufficient for our purposes.
//
//  - Could we weak-link some potentially unimplemented functions such as the
//    pre/post frame hooks?
///

#ifndef FSINTERFACE_H
#define FSINTERFACE_H

#include "fsTypes.h"

///
// @impl in frontend code.
//
// The definition there **must** not be a typedef.
///
typedef struct FSPSView FSPSView;

///
// Main game loop.
//
// This blocks until an appropriate end condition is meet.
///
void fsGameLoop(FSPSView *v, FSView *g);

///
// Open the specified file and attempt to parse any known options.
///
void fsParseIniFile(FSPSView *p, FSView *v, const char *fname);


///*********************/
// # Frontend Interface
//
// The following functions **must** be implemented by any frontend. If they are
// omitted, a linker error will occur.
//************/

///
// Return the current time with microsecond granularity.
//
// The reference clock should be monotonic.
///
FSLong fsiGetTime(FSPSView *v);

///
// Sleep for the specific number of microseconds.
///
void fsiSleepUs(FSPSView *v, FSLong time);

///
// Return the set of virtual keys that are currently pressed.
//
// The translation from physical keys to virtual keys must be handled by the
// frontend.
///
FSBits fsiReadKeys(FSPSView *v);

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
void fsiPlaySe(FSPSView *v, FSBits se);

///
// Try to register the specified key with the views keymap.
///
void fsiAddToKeymap(FSPSView *v, const int vkey, const char *key);

///
// Process an key-value pair option.
///
void fsiUnpackFrontendOption(FSPSView *v, const char *key, const char *value);

#endif

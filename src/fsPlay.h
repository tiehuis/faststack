// Defines the main game loop entry point and the required functions
// to be implemented as an interface.
//
// The definition of the FSPSView should have a pointer to a generic
// FSView instance so it can query the field state.

#ifndef FSPLAY_H
#define FSPLAY_H

#include "fsTypes.h"

typedef struct FSPSView FSPSView;

// Start the main game loop.
//
// Since we do not know the internal FSPSView structure, we require the
// generic version to be passed explicitly.
void fsPlayStart(FSPSView *v, FSView *g);

///
// The following defines the interface which MUST be implemented by any
// frontend.

// Return the clock time in microsecond granularity.
FSLong fsGetTime(FSPSView *v);

// Sleep for the specified number of microseconds.
void fsSleepUs(FSPSView *v, FSLong time);

// Return the set of virtual keys that were read from the physical device.
// This should also handle any other events that aren't explicitly keys.
FSBits fsReadKeys(FSPSView *v);

// Peform an entire draw, blit loop
void fsDraw(FSPSView *v);

// Run before any user code is processed in a tick
void fsPreFrameHook(FSPSView *v);

// Run after we have slept for the specified period of time
void fsPostFrameHook(FSPSView *v);

#endif

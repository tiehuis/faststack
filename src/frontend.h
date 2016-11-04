///
// frontend.h
// ============
//
// Core include for specifying which frontend is used statically.
///

#ifndef FS_FRONTEND_H
#define FS_FRONTEND_H

#ifdef FS_USE_SDL2
#include "frontend/SDL2/frontend.h"
#elif FS_USE_TERMINAL
#include "frontend/terminal/frontend.h"
#else
#error "No frontend selected!"
#endif

#endif

///
// finesse.h
// =========
//
// Header file for finesse functions.
///

#ifndef FINESSE_H
#define FINESSE_H

#include "core.h"

// Return the minimum number of movements and rotations for the specified
// piece at its position.
//
// Rotation is store in x and Movement in y.
i8x2 fsMinimalFinesseCount(FSBlock piece, i8 x, i8 theta);

#endif

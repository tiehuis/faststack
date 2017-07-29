///
// view.h
// ======
//
// Declaration of the FSView type. This acts as the generic containing
// structure for all sub-types.
///

#ifndef FS_VIEW_H
#define FS_VIEW_H

#include "core.h"

///
// A generic view of a games components.
//
// The 'FSEngine' instance does not handle all the components, such as input.
// This view encapsulates all these components into one structure.
///
struct FSView {
    /// Current game instance.
    FSEngine *game;

    /// Current input state.
    FSControl *control;

    /// Data Access object state.
    FSDao *dao;

    /// Is this a replay playback?
    bool replayPlayback;

    /// Filename of the replay to load
    char *replayName;
};

#endif

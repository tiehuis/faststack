///
// faststack.h
// ===========
//
// This exposes the entire public interface exposed by the `faststack` engine.
//
// Notes:
//  * A documented single header file would be ideal but is extra work for
//    increased maintenance cost.
///

#ifndef FASTSTACK_H
#define FASTSTACK_H

#include "config.h"
#include "control.h"
#include "core.h"
#include "default.h"
#include "engine.h"
#include "interface.h"
#include "internal.h"
#include "rand.h"
#include "rotation.h"
#include "view.h"

#ifndef FS_DISABLE_OPTION
#include "option.h"
#endif

#ifndef FS_DISABLE_LOG
#include "log.h"
#endif

#ifndef FS_DISABLE_HISCORE
#include "hiscore.h"
#endif

#ifndef FS_DISABLE_HISCORE
#include "replay.h"
#endif

#endif

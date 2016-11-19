///
// hiscore.h
// =========
//
// Hiscore related functionality.
///

#ifndef FS_HISCORE_H
#define FS_HISCORE_H

#include "core.h"

#ifndef FS_DISABLE_HISCORE

// Add an entry to the hiscore table
void fsHiscoreInsert(const FSEngine *f);

#else

#define fsHiscoreInsert(f)

#endif // FS_DISABLE_HISCORE

#endif

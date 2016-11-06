///
// fsConfig.h
// ==========
//
// Compile-Time configuration variables.
//
// Contains variables which typically specify how much stack memory to
// allocate for various structures. This can be configured based on the
// requirements of the frontend implementation.
//
// Note: Do not include this directly. 'fsEngine.h' includes this.
///

#ifndef FS_CONFIG_H
#define FS_CONFIG_H

/// Maximum height of a playfield.
//
//  * Constraints
//      FS_MAX_HEIGHT <= sizeof(u32) * 8
//
//      Due to the current algorithm used for clearing lines.
///
#define FS_MAX_HEIGHT 25

/// Maximum width of a playfield.
#define FS_MAX_WIDTH 20

/// Maximum size of preview piece buffer.
#define FS_PREVIEW_MAX 10

/// Maximum number of wallkick tables a rotation system is allowed
#define FS_MAX_KICK_LEN 10

/// Maximum number of preview pieces available. Clamps higher values.
#define FS_MAX_PREVIEW_COUNT 5

/// Maximum length of internal randomizer buffer.
#define FS_RAND_BUFFER_LEN (2 * 7)

/// Maximum number of wallkick tests allowed.
#define FS_MAX_NO_OF_WALLKICK_TABLES 10

/// Maximum number of keys an action can be triggered by.
#define FS_MAX_KEYS_PER_ACTION 3

/// Name of configuration file.
#define FS_CONFIG_FILENAME "fs.ini"

/// Name of the log file to write to.
#define FS_LOG_FILENAME "fs.log"

/// Name of the hiscore file to write do.
#define FS_HISCORE_FILENAME "fs.hiscore"

#endif // FS_CONFIG_H

///
// config.h
// ========
//
// Compile-Time configuration values.
//
// These variables configure compile-time sizes of certain arrays and specify
// the standard names for configuration files. These values can be configured
// depending on the specific frontend in some cases.
///

#ifndef FS_CONFIG_H
#define FS_CONFIG_H

// Maximum height of a playfield.
//
// Notes:
//  - This is currently constrained by an upper bound of 32 due to the
//    algorithm used for clearing lines.
#define FS_MAX_HEIGHT 25

// Maximum width of a playfield.
#define FS_MAX_WIDTH 20

// Maximum number of wallkick tests in a single rotation system.
#define FS_MAX_KICK_LEN 10

// Maximum size of preview piece buffer.
#define FS_MAX_PREVIEW_COUNT 5

// Maximum scratch space for internal randomizer buffer.
#define FS_RAND_BUFFER_LEN (2 * 7)

// Maximum number of wallkick tables per rotation system.
#define FS_MAX_NO_OF_WALLKICK_TABLES 10

// Maximum number of unique keys each action can be triggered by.
#define FS_MAX_KEYS_PER_ACTION 3

// FastStack configuration file name.
#define FS_CONFIG_FILENAME "fs.ini"

// FastStack log file name.
#define FS_LOG_FILENAME "fs.log"

// FastStack hiscore file name.
#define FS_HISCORE_FILENAME "fs.hiscore"

#endif // FS_CONFIG_H

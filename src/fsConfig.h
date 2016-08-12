///
// fsConfig.h
//
// Compile-Time configuration variables.
//
// Contains variables which typically specify how much stack memory to
// allocate for various structures. This can be configured based on the
// requirements of the frontend implementation.
//
// Note: Do not include this directly. 'fs.h' includes this.
///

/// Maximum height of a playfield.
//
//  - Constraints
//      FS_MAX_HEIGHT <= sizeof(FSBits) * 8
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

/// Maximum length of internal randomizer buffer.
#define FS_RAND_BUFFER_LEN (2 * 7)

/// Maximum number of wallkick tests allowed.
#define FS_MAX_NO_OF_WALLKICK_TABLES 5

/// Maximum number of keys an action can be triggered by.
#define FS_MAX_KEYS_PER_ACTION 3

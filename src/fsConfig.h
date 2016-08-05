// Compile-Time configuration variables.
//
// Do not include this header directly, since fs.h includes it.
// Reducing these in some cases can improve memory usage, but beware!

// The maximum possible height a playfield can be
#define FS_MAX_HEIGHT 25

// The maximum possible width a playfield can be
#define FS_MAX_WIDTH 20

// The maximum number of previews that can be stored
#define FS_PREVIEW_MAX 10

// The maximum number of wallkicks allowed in a table
#define FS_MAX_KICK_LEN 10

// The maximum length of an internal randomizer working bag
#define FS_RAND_BUFFER_LEN (2 * 7)

// How many unique wallkick tables a single rotation system can have
#define FS_MAX_NO_OF_WALLKICK_TABLES 5

// How many keys can be assigned to a single action i.e. z and x could both rotr
#define FS_MAX_KEYS_PER_ACTION 1

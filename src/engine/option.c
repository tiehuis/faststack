///
// option.c
// ========
//
// Handle parsing of configuration files and the associated setting of value
// within a `FSEngine` instance.
//
// We make heavy macro usage in order to get thorough input-checking for values
// across a number of types. Will likely be slightly adjusted if we move hasing
// approaches into here.
///

#ifndef FS_DISABLE_OPTION

#include "engine.h"
#include "dao.h"
#include "option.h"
#include "log.h"
#include "rotation.h"
#include "rand.h"
#include "view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// TODO: Expose in a slightly better way to the frontend. Perhaps just require
// a registration call for the following and otherwise default to NULL.
extern const char *fsiFrontendName;
void fsiUnpackFrontendOption(FSFrontend *v, const char *key, const char *value);
void fsiAddToKeymap(FSFrontend *v, const int vkey, const char *key, bool isDefault);

// Maximum values for input when parsing ini options
#define MAX_LINE_LENGTH 512
#define MAX_ID_LENGTH 32

int strcmpi(const char *a, const char *b)
{
    for (;; a++, b++) {
        const int d = tolower(*a) - tolower(*b);
        if (d || !*a) {
            return d;
        }
    }
}

#define M(x) !strcmpi(value, x)

static inline int fsRandomizerLookup(const char *value)
{
    if (M("simple") || M("0"))
        return FST_RAND_SIMPLE;
    else if (M("bag7") || M("1"))
        return FST_RAND_BAG7;
    else if (M("tgm1") || M("2"))
        return FST_RAND_TGM1;
    else if (M("tgm2") || M("3"))
        return FST_RAND_TGM2;
    else if (M("tgm3") || M("4"))
        return FST_RAND_TGM3;
    else if (M("bag7-seam") || M("5"))
        return FST_RAND_BAG7_SEAM_CHECK;
    else if (M("bag6") || M("6"))
        return FST_RAND_BAG6;
    else if (M("bag14") || M("7"))
        return FST_RAND_MULTI_BAG2;
    else if (M("bag28") || M("8"))
        return FST_RAND_MULTI_BAG4;
    else if (M("bag63") || M("9"))
        return FST_RAND_MULTI_BAG9;

    return -1;
}

static inline int fsRotationSystemLookup(const char *value)
{
    if (M("simple") || M("0"))
        return FST_ROTSYS_SIMPLE;
    else if (M("sega") || M("1"))
        return FST_ROTSYS_SEGA;
    else if (M("srs") || M("2"))
        return FST_ROTSYS_SRS;
    else if (M("arikasrs") || M("3"))
        return FST_ROTSYS_ARIKA_SRS;
    else if (M("tgm12") || M("4"))
        return FST_ROTSYS_TGM12;
    else if (M("tgm3") || M("5"))
        return FST_ROTSYS_TGM3;
    else if (M("dtet") || M("6"))
        return FST_ROTSYS_DTET;

    return -1;
}

static inline int fsLockStyleLookup(const char *value)
{
    if (M("entry") || M("0"))
        return FST_LOCK_ENTRY;
    if (M("step") || M("1"))
        return FST_LOCK_STEP;
    if (M("move") || M("2"))
        return FST_LOCK_MOVE;

    return -1;
}

static inline int fsInitialActionStyleLookup(const char *value)
{
    if (M("none") || M("0"))
        return FST_IA_NONE;
    if (M("persistent") || M("1"))
        return FST_IA_PERSISTENT;
    if (M("trigger") || M("2"))
        fsLogWarning("initialActionStyle = trigger is not implemented!");

    return -1;
}

///
// This function defines which option names are valid within an `ini` file.
//
// All keys are case-insensitive.
///
static void unpackOptionValue(struct FSFrontend *p, FSView *v, const char *k,
                              const char *value)
{
    if (!strncmp(k, "game.", 5)) {
        const char *key = k + 5;
        FSEngine *dst = v->game;

        TS_BOOL      (warnOnBadFinesse);
        TS_INT       (areDelay);
        TS_BOOL      (areCancellable);
        TS_INT       (dasSpeed);
        TS_INT       (dasDelay);
        TS_INT       (lockDelay);
        TS_INT_FUNC  (randomizer, fsRandomizerLookup);
        TS_INT_FUNC  (rotationSystem, fsRotationSystemLookup);
        TS_INT_RANGE (msPerTick, 1, INT_MAX);
        TS_INT_RANGE (ticksPerDraw, 1, INT_MAX);
        TS_INT_RANGE (fieldHidden, 0, FS_MAX_HEIGHT);
        TS_INT_RANGE (fieldHeight, 0, FS_MAX_HEIGHT);
        TS_INT_RANGE (fieldWidth, 0, FS_MAX_WIDTH);
        TS_INT_FUNC  (lockStyle, fsLockStyleLookup);
        TS_INT       (floorkickLimit);
        TS_BOOL      (infiniteReadyGoHold);
        TS_BOOL      (oneShotSoftDrop);
        TS_INT       (readyPhaseLength);
        TS_INT       (goPhaseLength);
        TS_INT       (nextPieceCount);
        TS_INT       (goal);
        TS_INT_RANGE (gravity, 0, INT_MAX);
        TS_INT_RANGE (softDropGravity, 0, INT_MAX);
        TS_INT_FUNC  (initialActionStyle, fsInitialActionStyleLookup);
    }
    else if (!strncmp(k, "keybind.", 8)) {
        const char *key = k + 8;

        TS_KEY       (rotateRight, FST_VK_ROTR);
        TS_KEY       (rotateLeft, FST_VK_ROTL);
        TS_KEY       (rotate180, FST_VK_ROTH);
        TS_KEY       (left, FST_VK_LEFT);
        TS_KEY       (right, FST_VK_RIGHT);
        TS_KEY       (down, FST_VK_DOWN);
        TS_KEY       (up, FST_VK_UP);
        TS_KEY       (hold, FST_VK_HOLD);
        TS_KEY       (quit, FST_VK_QUIT);
        TS_KEY       (restart, FST_VK_RESTART);
    }
    else if (!strncmp(k, "frontend.", 9)) {
        const size_t slen = strlen(fsiFrontendName);
        if (!strncmp(k + 9, fsiFrontendName, slen)) {
            fsiUnpackFrontendOption(p, k + 9 + slen + 1, value);
        }

        // If this was an option for another frontend, we should silently omit
        // it from our processing.
        return;
    }

    fsLogWarning("No suitable key found for option %s = %s", k, value);
}

const char *usage =
"faststack [-hiv]\n"
"\n"
"Options:\n"
"   -h --help       Display this message and quit\n"
"   -i --no-ini     Do not load options from the configuration file\n"
"   -v              Increase the logging level\n";

///
// Parse a command-line argument string.
//
// Notes:
//
//  * Consider a generic getopt implementation and handling in the actual main
//    function instead?
//
//  * Potential options/commands that may be added:
//      - `replay [filename]`
///
void fsParseOptString(FSOptions *o, int argc, char **argv)
{
    memset(o, 0, sizeof(FSOptions));

    for (int i = 1; i < argc; ++i) {
        const char *opt = argv[i];

        if (!strcmp("-v", opt)) {
            o->verbosity = FS_LOG_LEVEL_INFO;
        }
        else if (!strcmp("-vv", opt)) {
            o->verbosity = FS_LOG_LEVEL_DEBUG;
        }
        else if (!strcmp("-i", opt) || !strcmp("--no-ini", opt)) {
            o->no_ini = true;
        }
        else if (!strcmp("-h", opt) || !strcmp("--help", opt)) {
            printf("%s\n", usage);
            exit(0);
        }
        else if (!strcmp("--db-path", opt)) {
            printf("%s\n", daoGetDatabasePath());
            exit(0);
        }
        else if (strncmp("-", opt, 1) && strncmp("--", opt, 2)) {
            // Non-option argument is a replay (take last)
            o->replay = (char*) opt;
        }
        else {
            printf("Unknown argument: %s\n", argv[i]);
            exit(1);
        }
    }
}

///
// Parse an ini file into the specified view states.
//
// Format
// ======
//
//  * Comments must appears at the start of the line (excluding whitespace).
//
//  * Invalid keys and values are warned and skipped.
//
//  * Multiple values can be specified for a single key. These are
//    comma-seperated and will be treated as successive individual
//    key-value pairs.
//
//  * The last value encountered will be the one that is usually set.
//    Exceptions for multi-valued items like keybindings.
//
//  * The maximum length of a group and key is 64 bytes.
//
//  * The maximum length of a value is 32 bytes.
//
//  * The maximum line length is 512 bytes.
//
// Example
// =======
//
// ```
// [meta]
// ; A comment
// key = value
// multi_valued_key = item1, item2, item3
// ```
//
// Will parse into the following key-value pairs:
//
// ```
// meta.key, value
// meta.multi_valued_key, item1
// meta.multi_valued_key, item2
// meta.multi_valued_key, item3
// ```
///

/// Consume non-empty characters until the specified is found.
static inline int eat_till(char **s, const char c)
{
    int count = 0;
    while (**s && !isspace(**s) && **s != c) {
        count++, (*s)++;
    }
    return count;
}

/// Consume all empty characters.
static inline void eat_space(char **s)
{
    while (**s && isspace(**s)) {
        (*s)++;
    }
}

void fsParseIniFile(struct FSFrontend *p, FSView *v, const char *fname)
{
    char buffer[MAX_LINE_LENGTH];
    int optionsCounted, c;

    FILE *fd = fopen(fname, "r");
    if (!fd) {
        fsLogWarning("Failed to open ini file: %s.", fname);
        fsLogWarning("Falling back to defaults");
        return;
    }

    // `group.key` segment.
    char groupKey[2 * MAX_ID_LENGTH] = {0};

    // Pointer to `key` segment.
    char *keySegment = groupKey;

    char value[MAX_ID_LENGTH] = {0};
    int line = 0;

    while (fgets(buffer, MAX_LINE_LENGTH, fd)) {
        char *s = buffer;
        eat_space(&s);
        line += 1;

        switch (*s) {
          case '[':
            // Expect '['
            s++;

            eat_space(&s);
            c = eat_till(&s, ']');
            memcpy(groupKey, s - c, c);

            // Group and key are seperated by a '.' EXCEPT when the group is
            // empty (unspecified or cleared with [])
            if (c != 0) {
                groupKey[c] = '.';
                keySegment = groupKey + c + 1;
            }
            else {
                keySegment = groupKey;
            }
            break;

          case ';':
          case '\0':
            break;

          default:
            c = eat_till(&s, '=');
            memcpy(keySegment, s - c, c);
            keySegment[c] = 0;
            eat_space(&s);

            // Expect '='
            if (*s++ != '=') {
                fsLogWarning("line %d: Key %s missing '=' symbol", line, keySegment);
                break;
            }

            // Unpack all values in a comma-seperated list. A trailing comma
            // is not an error.
            optionsCounted = 0;
            while (*s != '\0') {
                eat_space(&s);

                // Skip comma from previous key.
                if (*s == ',') {
                    if (optionsCounted == 0) {
                        fsLogWarning("line %d: Comma seen before a value", line);
                    }
                    s++;
                }

                c = eat_till(&s, ',');

                // Ignore empty key section. Spacing around comma.
                if (!c || !*s) {
                    continue;
                }

                memcpy(value, s - c, c);
                value[c] = 0;
                unpackOptionValue(p, v, groupKey, value);
                optionsCounted++;
            }

            if (optionsCounted == 0) {
                fsLogWarning("line %d: Key %s has no value", line, keySegment);
            }

            break;
        }
    }

    fclose(fd);
}

///
// TODO: Should use platform-specific access functions.
int fileExists(const char *path)
{
    FILE *fd = fopen(path, "r");
    if (fd) {
        fclose(fd);
        return 1;
    } else {
        return 0;
    }
}

///
// Resolves which ini file we are loading.
//
// The load priority is as follows:
//  - fs.ini (current directory)
//  - $XDG_CONFIG_HOME/faststack/config.ini (linux)
const char* getIniFilePath(void)
{
    if (fileExists(FS_CONFIG_FILENAME)) {
        return FS_CONFIG_FILENAME;
    }

#ifdef __linux__
    const char *faststackConfig = "faststack/config.ini";

    // XDG_CONFIG_HOME incorporates the HOME folder by default so we need to
    // handle the case independently.
    char *xdgPath = getenv("XDG_CONFIG_HOME");

    static char iniPath[128];
    if (xdgPath) {
        snprintf(iniPath, sizeof(iniPath), "%s/%s", xdgPath, faststackConfig);
    } else {
        char *homePath = getenv("HOME");
        if (!homePath) {
            // TODO: Unsure about how good this fallback value is
            homePath = "~";
        }

        snprintf(iniPath, sizeof(iniPath), "%s/%s/%s",
                 homePath, ".config", faststackConfig);
    }

    return iniPath;
#else
    return NULL;
#endif
}

void fsTryParseIniFile(struct FSFrontend *p, FSView *v)
{
    const char *iniPath = getIniFilePath();
    if (iniPath) {
        fsLogInfo("loading config file from %s", iniPath);
        fsParseIniFile(p, v, iniPath);
    } else {
        fsLogInfo("no configuration file found");
    }
}

#endif

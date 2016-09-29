///
// fsOption.c
// ==========
//
// Handle parsing of configuration files and the associated setting of value
// within a `FSGame` instance.
//
// We make heavy macro usage in order to get thorough input-checking for values
// across a number of types. Will likely be slightly adjusted if we move hasing
// approaches into here.
///

#include "fs.h"
#include "fsInternal.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>

// Maximum values for input when parsing ini options
#define MAX_LINE_LENGTH 512
#define MAX_ID_LENGTH 32

///
// This variable must be present in the frontend.
///
extern const char *fsiFrontendName;

///
// Implementation in frontend code.
// Declaration in `fsInterface.h`.
///
struct FSPSView;
void fsiUnpackFrontendOption(struct FSPSView *v, const char *key, const char *value);
void fsiAddToKeymap(struct FSPSView *v, const int vkey, const char *key);

int strcmpi(const char *a, const char *b)
{
    for (;; a++, b++) {
        const int d = tolower(*a) - tolower(*b);
        if (d || !*a) {
            return d;
        }
    }
}

static inline int fsRandomizerLookup(const char *value)
{
    if (!strcmpi(value, "simple"))
        return FST_RAND_SIMPLE;
    else if (!strcmpi(value, "noszobag7"))
        return FST_RAND_NOSZO_BAG7;
    else if (!strcmpi(value, "tgm1"))
        return FST_RAND_TGM1;
    else if (!strcmpi(value, "tgm2"))
        return FST_RAND_TGM2;

    return -1;
}

static inline int fsRotationSystemLookup(const char *value)
{
    if (!strcmpi(value, "simple"))
        return FST_ROTSYS_SIMPLE;
    else if (!strcmpi(value, "sega"))
        return FST_ROTSYS_SEGA;
    else if (!strcmpi(value, "srs"))
        return FST_ROTSYS_SRS;
    else if (!strcmpi(value, "arikasrs"))
        return FST_ROTSYS_ARIKA_SRS;
    else if (!strcmpi(value, "tgm12"))
        return FST_ROTSYS_TGM12;
    else if (!strcmpi(value, "tgm3"))
        return FST_ROTSYS_TGM3;
    else if (!strcmpi(value, "dtet"))
        return FST_ROTSYS_DTET;

    return -1;
}

static inline int fsLockStyleLookup(const char *value)
{
    if (!strcmpi(value, "entry"))
        return FST_LOCK_ENTRY;
    if (!strcmpi(value, "step"))
        return FST_LOCK_STEP;
    if (!strcmpi(value, "move"))
        return FST_LOCK_MOVE;

    return -1;
}

static inline int fsInitialActionStyleLookup(const char *value)
{
    if (!strcmpi(value, "none"))
        return FST_IA_NONE;
    if (!strcmpi(value, "persistent"))
        return FST_IA_PERSISTENT;
    if (!strcmpi(value, "trigger"))
        fsLogWarning("initialActionStyle = trigger is not implemented!");

    return -1;
}

///
// This function defines which option names are valid within an `ini` file.
//
// All keys are case-insensitive.
///
static void unpackOptionValue(struct FSPSView *p, FSView *v, const char *k,
                              const char *value)
{
    if (!strncmp(k, "game.", 5)) {
        const char *key = k + 5;
        FSGame *dst = v->game;

        TS_INT       (areDelay);
        TS_BOOL      (areCancellable);
        TS_INT       (lockDelay);
        TS_INT_FUNC  (randomizer, fsRandomizerLookup);
        TS_INT_FUNC  (rotationSystem, fsRotationSystemLookup);
        TS_INT_RANGE (msPerTick, 1, INT_MAX);
        TS_INT_RANGE (ticksPerDraw, 1, INT_MAX);
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
        TS_FLT       (gravity);
        TS_FLT       (softDropGravity);
        TS_INT_FUNC  (initialActionStyle, fsInitialActionStyleLookup);
    }
    else if (!strncmp(k, "control.", 8)) {
        const char *key = k + 8;
        FSControl *dst = v->control;

        TS_INT       (dasSpeed);
        TS_INT       (dasDelay);
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
    else {
        char buffer[2 * MAX_ID_LENGTH] = "frontend.";
        strncat(buffer, fsiFrontendName, 2 * MAX_ID_LENGTH - 9);
        const size_t slen = strlen(buffer);

        // If we encounter a frontend-defined option do not warn that no key
        // was found if nothing is parsed. Let the frontend manage this if
        // it needs to.
        if (!strncmp(k, buffer, slen)) {
            fsiUnpackFrontendOption(p, k + slen + 1, value);
            return;
        }
    }

    fsLogWarning("No suitable key found for option %s = %s", k, value);
}

///
// Parse a command-line argument string.
//
// Do we want this generic or just specify the options here?
// Probably generic, really.
///
void fsParseOptString(int argc, char **argv);

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

void fsParseIniFile(struct FSPSView *p, FSView *v, const char *fname)
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

///
// fsOption.c
//
// Handle parsing of configuration files and the associated setting of value
// within a `FSGame` instance.
//
// We make heavy macro usage in order to get thorough input-checking for values
// across a number of types. Will likely be slightly adjusted if we move hasing
// approaches into here.
///

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include "fs.h"
#include "fsLog.h"
#include "fsInternal.h"

///
// @impl in frontend code.
// @decl in `fsInterface.h`.
///
struct FSPSView;
void fsiUnpackFrontendOption(struct FSPSView *v, const char *key, const char *value);
void fsiAddToKeymap(struct FSPSView *v, const int vkey, const char *key);

/// Helper: Case insensitive strcmp for configuration parsing.
static int strcmpi(const char *a, const char *b)
{
    for (;; a++, b++) {
        const int d = tolower(*a) - tolower(*b);
        if (d || !*a)
            return d;
    }
}

///
// The following macros provide more robust parsing of key-value pairs into
// their associated variables.
//
// These make a lot of assumptions about variable names that are in scope, but
// adding any new values should be much easier now.
//
// They are summarised as follows:
//
//  TS_INT       - The value should be a positive integer.
//  TS_INT_RANGE - The value should be an integer within the specified range.
//  TS_FLT       - The value should be a positive float.
//  TS_FLT_RANGE - The value should be a float within the specified range.
//  TS_BOOL      - The value should be a boolean or equivalent.
//  TS_INT_FUNC  - The value should be an integer after querying a user function.
//
//  We assume the following:
//    - If we find a key match, there will be nothing else to do after assignment
//    - Positive values are expected by default
//    - `dst` is a pointer to a struct which contains the _id in question
//    - `key` will store the key we are checking
//    - `value` will store the value associated with this key
///

#define TS_INT(_id) TS_INT_RANGE(_id, 0, LLONG_MAX)

#define TS_INT_RANGE(_id, _lo, _hi)                                             \
do {                                                                            \
    if (!strcmpi(#_id, key)) {                                                  \
        errno = 0;                                                              \
        char *_endptr;                                                          \
        const long long _ival = strtoll(value, &_endptr, 10);                   \
                                                                                \
        if (errno == ERANGE) {                                                  \
            fsLogWarning("Ignoring %s since it does not fit in an integer", value);\
        }                                                                       \
        else if (_endptr == value) {                                            \
            fsLogError("Internal error: Found zero-length option value for %s", key);\
        }                                                                       \
        else {                                                                  \
            if (*_endptr != '\0') {                                             \
                fsLogWarning("Ignoring %s since it contains trailing garbage", value);\
            }                                                                   \
            else if (_ival < _lo || _hi < _ival) {                              \
                fsLogWarning("Ignoring %s since it is not in allowed range [%lld, %lld]",\
                        value, _lo, _hi);                                       \
            }                                                                   \
            else if (ceil(log2(llabs(_ival))) > 8 * sizeof(dst->_id) - 1) {     \
                fsLogWarning("Ignoring %s since it requires %d bits to represent"\
                             " when target requires %d",                        \
                             ceil(log2(llabs(_ival))), 8 * sizeof(dst->_id) -1);\
            }                                                                   \
            else {                                                              \
                dst->_id = _ival;                                               \
            }                                                                   \
        }                                                                       \
                                                                                \
        return;                                                                 \
    }                                                                           \
} while (0)

#define TS_INT_FUNC(_id, _func)                                                 \
do {                                                                            \
    if (!strcmpi(#_id, key)) {                                                  \
        const int _ival = _func(value);                                         \
                                                                                \
        if (_ival == -1) {                                                      \
            fsLogWarning("Ignoring unknown value %s for key %s", value, key);   \
        }                                                                       \
        else {                                                                  \
            dst->_id = _ival;                                                   \
        }                                                                       \
                                                                                \
        return;                                                                 \
    }                                                                           \
} while (0)

#define TS_FLT(_id) TS_FLT_RANGE(_id, 0.0, DBL_MAX)

#define TS_FLT_RANGE(_id, _lo, _hi)                                             \
do {                                                                            \
    if (!strcmpi(#_id, key)) {                                                  \
        errno = 0;                                                              \
        char *_endptr;                                                          \
        const double _ival = strtod(value, &_endptr);                           \
                                                                                \
        if (errno == ERANGE) {                                                  \
            fsLogWarning("Ignoring %s since it does not fit in a double", value);\
        }                                                                       \
        else if (_endptr == value) {                                            \
            fsLogError("Internal error: Found zero-length option value for %s", value);\
        }                                                                       \
        else {                                                                  \
            if (*_endptr != '\0') {                                             \
                fsLogWarning("Ignoring %s since it contains trailing garbage", value);\
            }                                                                   \
            else if (!isnormal(_ival)) {                                        \
                fsLogWarning("Ignoring non-normal floating-point value of %s", value);\
            }                                                                   \
            else if (_ival < _lo || _hi < _ival) {                              \
                fsLogWarning("Ignoring %s since it is not in allowed range [%lf, %lf]",\
                        value, _lo, _hi);                                       \
            }                                                                   \
            else {                                                              \
                dst->_id = _ival;                                               \
            }                                                                   \
        }                                                                       \
                                                                                \
        return;                                                                 \
    }                                                                           \
} while (0)

#define TS_BOOL(_id)                                                            \
do {                                                                            \
    if (!strcmpi(#_id, key)) {                                                  \
        if (!strcmpi(value, "true") || !strcmpi(value, "yes") || !strcmpi(value, "1"))\
            dst->_id = true;                                                    \
        else if (!strcmpi(value, "false") || !strcmpi(value, "no") || !strcmpi(value, "0"))\
            dst->_id = false;                                                   \
        else                                                                    \
            fsLogWarning("Invalid boolean value encountered %s", value);        \
                                                                                \
        return;                                                                 \
    }                                                                           \
} while (0)

#define TS_KEY(_id, _vkey)                                                      \
do {                                                                            \
    if (!strcmpi(#_id, key)) {                                                  \
        fsiAddToKeymap(p, _vkey, value);                                        \
        return;                                                                 \
    }                                                                           \
} while (0)

// Convert a string representation of a randomizer to its symbolic constant
static inline int fsRandomizerLookup(const char *value)
{
    if (!strcmpi(value, "simple"))
        return FSRAND_SIMPLE;
    else if (!strcmpi(value, "noszobag7"))
        return FSRAND_NOSZO_BAG7;
    else if (!strcmpi(value, "tgm1"))
        return FSRAND_TGM1;
    else if (!strcmpi(value, "tgm2"))
        return FSRAND_TGM2;

    return -1;
}

// Convert a string representation of a rotation system to its
// symbolic constant.
static inline int fsRotationSystemLookup(const char *value)
{
    if (!strcmpi(value, "simple"))
        return FSROT_SIMPLE;
    else if (!strcmpi(value, "srs"))
        return FSROT_SRS;
    else if (!strcmpi(value, "arikasrs"))
        return FSROT_ARIKA_SRS;
    else if (!strcmpi(value, "tgm12"))
        return FSROT_TGM12;
    else if (!strcmpi(value, "dtet"))
        return FSROT_DTET;

    return -1;
}

// Convert a string representation of a lock style to its symbolic constant.
static inline int fsLockStyleLookup(const char *value)
{
    if (!strcmpi(value, "entry"))
        return FSLOCK_ENTRY;
    if (!strcmpi(value, "step"))
        return FSLOCK_STEP;
    if (!strcmpi(value, "move"))
        return FSLOCK_MOVE;

    return -1;
}

// Convert a string representation of a initial action style to its symbolic constant.
static inline int fsInitialActionStyleLookup(const char *value)
{
    if (!strcmpi(value, "none"))
        return FSIA_NONE;
    if (!strcmpi(value, "persistent"))
        return FSIA_PERSISTENT;
    if (!strcmpi(value, "trigger"))
        fsLogWarning("initialActionStyle = trigger is not implemented!");

    return -1;
}

// This is where option names are implicitly defined as encountered in
// configuration files.
//
// All keys are case-insensitive.
struct FSPSView;
static void unpackOptionValue(struct FSPSView *p, FSView *v, const char *k, const char *value)
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

        TS_KEY       (rotateRight, VKEYI_ROTR);
        TS_KEY       (rotateLeft, VKEYI_ROTL);
        TS_KEY       (rotate180, VKEYI_ROTH);
        TS_KEY       (left, VKEYI_LEFT);
        TS_KEY       (right, VKEYI_RIGHT);
        TS_KEY       (down, VKEYI_DOWN);
        TS_KEY       (up, VKEYI_UP);
        TS_KEY       (hold, VKEYI_HOLD);
    }
    // Hardcoded currently, may have to require frontend to do this.
    // Can just extern a variable name and require it to be implemented by frontend.
    else if (!strncmp(k, "frontend.sdl2", 13)) {
        fsiUnpackFrontendOption(p, k + 13, value);

        // Handle non-existent options at the platform level
        return;
    }

    fsLogWarning("No suitable key found for option %s = %s", k, value);
}

///
// Parse an ini file into the specified view states.
//
// # Format
//
//  - Comments must appears at the start of the line (excluding whitespace).
//
//  - Invalid keys and values are warned and skipped.
//
//  - Multiple values can be specified for a single key. These are
//    comma-seperated and will be treated as successive individual
//    key-value pairs.
//
//  - The last value encountered will be the one that is usually set.
//    Exceptions for multi-valued items like keybindings.
//
//  - The maximum length of a group and key is 64 bytes.
//
//  - The maximum length of a value is 32 bytes.
//
//  - The maximum line length is 512 bytes.
//
// # Example
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

// Assume a line is at most 512 bytes long
#define MAX_LINE_LENGTH 512
#define MAX_ID_LENGTH 32

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
    while (**s && isspace(**s)) (*s)++;
}

/// Perform the actual parsing of the file.
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

    // group.key tag buffer
    char groupKey[2 * MAX_ID_LENGTH] = {0};

    // Key segment with groupKey buffer
    char *keySegment = groupKey;

    // Value
    char value[MAX_ID_LENGTH] = {0};

    // This is a very rudimentary line count.
    int line = 0;

    while (fgets(buffer, MAX_LINE_LENGTH, fd)) {
        char *s = buffer;
        eat_space(&s);
        line += 1;

        switch (*s) {
          case '[':
            // Skip pending '['
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

            // Unpack all values in a comma-seperated list.
            // A trailing comma is not an error.
            optionsCounted = 0;
            while (*s != '\0') {
                eat_space(&s);

                // Skip comma from previous key.
                if (*s == ',') {
                    if (optionsCounted == 0)
                        fsLogWarning("line %d: Comma seen before a value", line);
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

            // Warn on no value present for key
            if (optionsCounted == 0) {
                fsLogWarning("line %d: Key %s has no value", line, keySegment);
            }

            break;
        }
    }

    fclose(fd);
}

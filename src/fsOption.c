///
// fsOption.c
//
// Handle parsing of configuration files and the associated setting of value
// within a `FSGame` instance.
///

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// This is where option names are implicitly defined as encountered in
// configuration files.
//
// All keys are case-insensitive.
struct FSPSView;
static void unpackOptionValue(struct FSPSView *p, FSView *v, const char *key, const char *value)
{
    if (!strncmp(key, "game.", 5)) {
        const char *s = key + 5;
        FSGame *f = v->game;

        if (!strcmpi(s, "areDelay"))
            f->areDelay = atol(value);
        else if (!strcmpi(s, "lockDelay"))
            f->lockDelay = atol(value);
        else if (!strcmpi(s, "randomizer"))
            f->randomizer = fsRandomizerLookup(value);
        else if (!strcmpi(s, "rotationSystem"))
            f->rotationSystem = fsRotationSystemLookup(value);
        else if (!strcmpi(s, "msPerTick"))
            f->msPerTick = atol(value);
        else if (!strcmpi(s, "msPerDraw"))
            f->msPerDraw = atol(value);
        else if (!strcmpi(s, "fieldHeight"))
            f->fieldHeight = atol(value);
        else if (!strcmpi(s, "fieldWidth"))
            f->fieldWidth = atol(value);
        else if (!strcmpi(s, "lockStyle"))
            f->lockStyle = fsLockStyleLookup(value);
        else if (!strcmpi(s, "infiniteReadyGoHold"))
            f->infiniteReadyGoHold = (bool) atol(value);
        else if (!strcmpi(s, "readyPhaseLength"))
            f->readyPhaseLength = atol(value);
        else if (!strcmpi(s, "goPhaseLength"))
            f->goPhaseLength = atol(value);
        else if (!strcmpi(s, "nextPieceCount"))
            f->nextPieceCount = atol(value);
        else if (!strcmpi(s, "goal"))
            f->goal = atol(value);
    }
    else if (!strncmp(key, "control.", 8)) {
        const char *s = key + 8;
        FSControl *c = v->control;

        if (!strcmpi(s, "dasSpeed"))
            c->dasSpeed = atol(value);
        else if (!strcmpi(s, "dasDelay"))
            c->dasDelay = atol(value);
    }
    else if (!strncmp(key, "keybind.", 8)) {
        const char *s = key + 8;

        if (!strcmpi("rotateRight", s))
            fsiAddToKeymap(p, VKEYI_ROTR, value);
        else if (!strcmpi("rotateLeft", s))
            fsiAddToKeymap(p, VKEYI_ROTL, value);
        else if (!strcmpi("rotate180", s))
            fsiAddToKeymap(p, VKEYI_ROTH, value);
        else if (!strcmpi("left", s))
            fsiAddToKeymap(p, VKEYI_LEFT, value);
        else if (!strcmpi("right", s))
            fsiAddToKeymap(p, VKEYI_RIGHT, value);
        else if (!strcmpi("down", s))
            fsiAddToKeymap(p, VKEYI_DOWN, value);
        else if (!strcmpi("up", s))
            fsiAddToKeymap(p, VKEYI_UP, value);
        else if (!strcmpi("hold", s))
            fsiAddToKeymap(p, VKEYI_HOLD, value);
    }
    // Hardcoded currently, may have to require frontend to do this
    else if (!strncmp(key, "frontend.sdl2", 13)) {
        fsiUnpackFrontendOption(p, key + 13, value);
    }
}

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

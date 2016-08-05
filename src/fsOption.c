// Handle reading options files and copying options into the specified
// structs.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "fsInternal.h"

// Case insensitive strcmp.
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
    if (!strcmpi(value, "noszobag7"))
        return FSRAND_NOSZO_BAG7;

    return -1;
}

// Convert a string representation of a rotation system to its
// symbolic constant.
static inline int fsRotationSystemLookup(const char *value)
{
    if (!strcmpi(value, "simple"))
        return FSROT_SIMPLE;
    if (!strcmpi(value, "srs"))
        return FSROT_SRS;
    if (!strcmpi(value, "arikasrs"))
        return FSROT_ARIKA_SRS;
    if (!strcmpi(value, "tgm12"))
        return FSROT_TGM12;
    if (!strcmpi(value, "dtet"))
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

// This is where option names are implicitly defined as encountered in ini
// files. All names are case-insensitive.
static void unpackOptionValue(FSView *v, const char *key, const char *value)
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
    }
    else if (!strncmp(key, "control.", 8)) {
        const char *s = key + 8;
        FSControl *c = v->control;

        if (!strcmpi(s, "dasSpeed"))
            c->dasSpeed = atol(value);
        else if (!strcmpi(s, "dasDelay"))
            c->dasDelay = atol(value);
    }
}

// Assume a line is at most 512 bytes long
#define MAX_LINE_LENGTH 512
#define MAX_ID_LENGTH 32

// Read an ini file into an option lookup table
//
// INI files are of the following form:
// ```
// [group]
// ; comment
// key = value
// ```
//
// Comments must occur at the start of the line. Empty lines
// are ignored.
// Errors are ignored silently.
void fsParseIniFile(FSView *v, const char *fname)
{
    char buffer[MAX_LINE_LENGTH];

    FILE *fd = fopen(fname, "r");
    if (!fd) {
        fprintf(stderr, "Failed to open init file: %s\n", fname);
        exit(2);
    }

    // [group] tag
    char grp[MAX_ID_LENGTH] = {0};
    char grpkey[MAX_ID_LENGTH * 2] = {0};

    // key = value
    char key[MAX_ID_LENGTH] = {0};
    char val[MAX_ID_LENGTH] = {0};

    // Read line by line, truncating any line that is too long
    while (fgets(buffer, MAX_LINE_LENGTH, fd)) {
        char *s = buffer;

        // Filter leading spaces
        while (*s && isspace(*s))
            s++;

        // Group case
        if (*s == '[') {
            s++;

            while (*s && isspace(*s)) {
                s++;
            }

            // Read the interior key until the next space
            int c = 0;
            while (*s && !isspace(*s) && *s != ']') {
                s++;
                c++;
            }

            // Read the key, copy into group buffer
            strncpy(grp, s - c, c);
            grp[c] = 0;
        }
        // Comment case
        else if (*s == ';') {
            // Do nothing
        }
        // Empty line
        else if (*s == '\0') {
            // Do nothing
        }
        // Key = value
        else {
            // Read till space or '='
            int c = 0;
            while (*s && !isspace(*s) && *s != '=') {
                s++;
                c++;
            }

            // Copy the value into a key
            strncpy(key, s - c, c);
            key[c] = 0;

            // Skip spaces
            while (*s && isspace(*s))
                s++;

            // Expect '='
            if (*s++ != '=') {
                fprintf(stderr, "Key %s missing a value\n", key);
                continue;
            }

            // Read the value
            while (*s && isspace(*s))
                s++;

            c = 0;
            while (*s && !isspace(*s)) {
                s++;
                c++;
            }

            strncpy(val, s - c, c);
            val[c] = 0;

            // Try an unpack the key-value pair into the current view
            snprintf(grpkey, 2 * MAX_ID_LENGTH, "%s.%s", grp, key);
            unpackOptionValue(v, grpkey, val);
        }
    }

    fclose(fd);
}

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
//
// Note: Should combine group and key buffers to avoid the copy.
void fsParseIniFile(struct FSPSView *p, FSView *v, const char *fname)
{
    char buffer[MAX_LINE_LENGTH];

    FILE *fd = fopen(fname, "r");
    if (!fd) {
        fsLogWarning("Failed to open .ini file. Falling back to defaults");
        return;
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
                fsLogWarning("Skipping key '%s' with no value", key);
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

            // If key is empty value didn't exist
            if (!c) {
                fsLogWarning("Skipping key '%s' with no value", key);
                continue;
            }

            strncpy(val, s - c, c);
            val[c] = 0;

            // Try an unpack the key-value pair into the current view
            snprintf(grpkey, 2 * MAX_ID_LENGTH, "%s.%s", grp, key);
            unpackOptionValue(p, v, grpkey, val);
        }
    }

    fclose(fd);
}

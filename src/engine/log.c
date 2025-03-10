///
// log.c
// =====
//
// Logging functions which can have their level changed at runtime.
//
// NOTE: Not thread-safe.
///

#ifndef FS_DISABLE_LOG

// TODO(#39): Platform-specific for `isatty` and `fileno`
#define _XOPEN_SOURCE 500
#include <unistd.h>

#include "core.h"
#include "log.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// TODO: Adjust how we disable features to avoid this

///
// Global variable used to filter which messages are printed.
///
static int fsCurrentLogLevel = FS_LOG_LEVEL_WARNING;

///
// Global variable for which stream to logging.
///
static FILE *fsLogStream = NULL;

// Is this a file our stream?
static bool using_file = false;

// Was anything written at all?
static bool wrote_data = false;

// Name of the log file. We don't use a static buffer here since every filename
// is expected to be a string literal which is stored in static memory anyway
// according to the C standard.
static char *fsLogFilename = NULL;

///
// Return a string with the current time.
//
// The string is invalidated on the next call to this function.
///
static char* ctimeStr(void)
{
    static char buffer[26];
    time_t timer;
    struct tm *tmInfo;

    time(&timer);
    tmInfo = localtime(&timer);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", tmInfo);
    return buffer;
}

///
// Return a string representation of a logging level.
///
static const char* logLevelStr(int level)
{
    static const char *logLevelNames[] = {
        "debug", "info", "warning", "error", "fatal"
    };

    return logLevelNames[level - 1];
}

///
// Return the vt100 colour code of this level.
///
static int logLevelColorCode(int level)
{
    static const int logLevelColors[] = {
        39, 37, 33, 31, 31
    };

    return logLevelColors[level - 1];
}

///
// Set the output file used for logging. stdout by default.
//
// The value of `name` MUST be a string literal or have static storage duration.
///
void fsSetLogFile(const char *name)
{
    if (name[0] == '-' && name[1] == '\0') {
        fsLogStream = stderr;
        using_file = false;
    }
    else {
        // We need to store the filename specified
        using_file = true;
        // This value will not be edited so ignore non-const cast
        fsLogFilename = (char*) name;
        FILE *fd = fopen(name, "w+");
        if (fd == NULL) {
            fsLogError("failed to use file output. Falling back to stderr");
            fsLogStream = stderr;
            using_file = false;
        }
        else {
            fsLogStream = fd;
            using_file = true;
        }
    }

}

///
// Set the logging level used.
///
void fsSetLogLevel(int level)
{
    fsCurrentLogLevel = level;
}

/// Only does something if this is an actual file (not stderr)
void fsCloseLogFile(void)
{
    if (using_file) {
        fclose(fsLogStream);

        if (!wrote_data) {
            if (remove(fsLogFilename)) {
                fsLogError("failed to remove empty log: %s", strerror(errno));
            }
        }
    }
}

///
// Main logging function.
///
void fsLog(int level, ...)
{
    if (level >= fsCurrentLogLevel) {
        wrote_data = true;

        int stat = isatty(fileno(fsLogStream));

        if (stat) {
            fprintf(fsLogStream, "\033[%dm", logLevelColorCode(level));
        }

        fprintf(fsLogStream, "[%s] [%s]: ", ctimeStr(), logLevelStr(level));

        if (stat) {
            fprintf(fsLogStream, "\033[0m");
        }

        va_list args;
        va_start(args, level);
        const char *format = va_arg(args, const char*);
        vfprintf(fsLogStream, format, args);
        va_end(args);

        fprintf(fsLogStream, "\n");
    }
}

#endif // FS_DISABLE_LOG

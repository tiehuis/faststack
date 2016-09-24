///
// fsLog.c
// =======
//
// Logging functions which can have their level changed at runtime.
//
// NOTE: Not thread-safe.
///

#include "fsLog.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

///
// Global variable used to filter which messages are printed.
///
int fsCurrentLogLevel = FS_LOG_LEVEL_INFO;

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

    return logLevelNames[level];
}

///
// Return the vt100 colour code of this level.
///
static int logLevelColorCode(int level)
{
    static const int logLevelColors[] = {
        39, 37, 33, 31, 31
    };

    return logLevelColors[level];
}

///
// Main logging function.
///
void fsLog(int level, ...)
{
    if (level >= fsCurrentLogLevel) {
        fprintf(stderr, "\033[%dm[%s] [%s]:\033[0m ",
                logLevelColorCode(level), ctimeStr(), logLevelStr(level));

        va_list args;
        va_start(args, level);
        const char *format = va_arg(args, const char*);
        vfprintf(stderr, format, args);
        va_end(args);

        fprintf(stderr, "\n");
    }
}

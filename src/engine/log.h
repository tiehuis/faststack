///
// log.h
// =====
//
// Header file for logging system.
//
// The logging system exposes two global variables. These are used to tweak the
// level and output stream which logging messages will be written to.
//
// This is an optional component. It can be turned off with the flag
// `FS_DISABLE_LOG`.
//
// Issues:
//  - Allow mock logging implementations to be used if `NDEBUG` is defined.
///

#ifndef FS_LOG_H
#define FS_LOG_H

enum LogLevel {
    FS_LOG_LEVEL_DEBUG = 1,
    FS_LOG_LEVEL_INFO,
    FS_LOG_LEVEL_WARNING,
    FS_LOG_LEVEL_ERROR,
    FS_LOG_LEVEL_FATAL
};

#define fsLogDebug(...) fsLog(FS_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define fsLogInfo(...) fsLog(FS_LOG_LEVEL_INFO, __VA_ARGS__)
#define fsLogWarning(...) fsLog(FS_LOG_LEVEL_WARNING, __VA_ARGS__)
#define fsLogError(...) fsLog(FS_LOG_LEVEL_ERROR, __VA_ARGS__)
#define fsLogFatal(...) fsLog(FS_LOG_LEVEL_FATAL, __VA_ARGS__)

#ifndef FS_DISABLE_LOG

void fsSetLogFile(const char *name);
void fsSetLogLevel(int level);
void fsCloseLogFile(void);

// Generic logging function. Prefer macro definitions instead.
void fsLog(int level, ...);

#else

#define fsSetLogFile(name)
#define fsSetLogLevel(level)
#define fsCloseLogFile()
#define fsLog(...)

#endif // FS_DISABLE_LOG

#endif // FS_LOG_H

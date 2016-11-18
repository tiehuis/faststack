///
// log.h
// =====
//
// Header file for logging system.
//
// The logging system exposes two global variables. These are used to tweak the
// level and output stream which logging messages will be written to.
//
// Issues:
//  - Allow mock logging implementations to be used if `NDEBUG` is defined.
///

#ifndef FS_LOG_H
#define FS_LOG_H

void fsSetLogFile(const char *name);
void fsSetLogLevel(int level);
void fsCloseLogFile(void);

enum LogLevel {
    FS_LOG_LEVEL_DEBUG = 1,
    FS_LOG_LEVEL_INFO,
    FS_LOG_LEVEL_WARNING,
    FS_LOG_LEVEL_ERROR,
    FS_LOG_LEVEL_FATAL
};

// Generic logging function. Prefer macro definitions instead.
void fsLog(int level, ...);

#define fsLogDebug(...) fsLog(FS_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define fsLogInfo(...) fsLog(FS_LOG_LEVEL_INFO, __VA_ARGS__)
#define fsLogWarning(...) fsLog(FS_LOG_LEVEL_WARNING, __VA_ARGS__)
#define fsLogError(...) fsLog(FS_LOG_LEVEL_ERROR, __VA_ARGS__)
#define fsLogFatal(...) fsLog(FS_LOG_LEVEL_FATAL, __VA_ARGS__)

#endif // FS_LOG_H

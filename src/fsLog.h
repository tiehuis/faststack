///
// fsLog.h
// =======
//
// Header file for logging messages.
///

#ifndef FS_LOG_H
#define FS_LOG_H

///
// We want to allow any client to set this variable to alter all logging.
///
extern int fsCurrentLogLevel;

///
// Represents a level of logging.
///
enum FS_LOG_LEVEL {
    /// Debug log message.
    FS_LOG_LEVEL_DEBUG = 1,

    /// Informational log message.
    FS_LOG_LEVEL_INFO,

    /// Warning log message.
    FS_LOG_LEVEL_WARNING,

    /// Error log message.
    FS_LOG_LEVEL_ERROR,

    /// Fatal log message.
    FS_LOG_LEVEL_FATAL
};

///
// Generic interface to log a message.
//
// This should not usually be used, prefer the specific defines.
///
void fsLog(int level, ...);

#define fsLogDebug(...) fsLog(FS_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define fsLogInfo(...) fsLog(FS_LOG_LEVEL_INFO, __VA_ARGS__)
#define fsLogWarning(...) fsLog(FS_LOG_LEVEL_WARNING, __VA_ARGS__)
#define fsLogError(...) fsLog(FS_LOG_LEVEL_ERROR, __VA_ARGS__)
#define fsLogFatal(...) fsLog(FS_LOG_LEVEL_FATAL, __VA_ARGS__)

#endif // FS_LOG_H

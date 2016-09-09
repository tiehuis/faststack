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
    FS_LOG_LEVEL_DEBUG,

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
void fsLog(int level, const char *format, ...);

#define fsLogDebug(format, ...) fsLog(FS_LOG_LEVEL_DEBUG, format, ## __VA_ARGS__)
#define fsLogInfo(format, ...) fsLog(FS_LOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define fsLogWarning(format, ...) fsLog(FS_LOG_LEVEL_WARNING, format, ## __VA_ARGS__)
#define fsLogError(format, ...) fsLog(FS_LOG_LEVEL_ERROR, format, ## __VA_ARGS__)
#define fsLogFatal(format, ...) fsLog(FS_LOG_LEVEL_FATAL, format, ## __VA_ARGS__)

#endif // FS_LOG_H

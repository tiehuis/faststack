///
// option.h
// ========
//
// Header file for option parsing. This exposes functions which perform
// command-line option parsing, ini file parsing and also helper methods
// for frontends performing their own option parsing as well.
//
// These functions are very assumptious, so read the documentation
// thoroughly!
///

#ifndef FS_OPTION_H
#define FS_OPTION_H

#include <stdbool.h>
#include <errno.h>
#include <float.h>
#include <limits.h>

// A struct containing command line option parameters for a generic faststack
// frontend implementation.
struct FSOptions {
    int verbosity;
    bool no_ini;
    char *replay;
};

int strcmpi(const char *a, const char *b);

void fsParseOptString(FSOptions *o, int argc, char **argv);

#ifndef FS_DISABLE_OPTION

void fsTryParseIniFile(FSFrontend *p, FSView *v);

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
//
//    * If we find a key match, there will be nothing else to do after assignment
//    * Positive values are expected by default
//    * `dst` is a pointer to a struct which contains the _id in question
//    * `key` will store the key we are checking
//    * `value` will store the value associated with this key
///

// Ceiling of the base 2 logarithm of x.
inline static u32 cilog2(long long x)
{
    int r = 1;
    while (x) {
        x >>= 1;
        r += 1;
    }
    return r;
}

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
            else if (_ival < (_lo) || (_hi) < _ival) {                          \
                fsLogWarning("Ignoring %s since it is not in allowed range [%lld, %lld]",\
                        value, _lo, _hi);                                       \
            }                                                                   \
            else if (cilog2(llabs(_ival)) > 8 * sizeof(dst->_id) - 1) {         \
                fsLogWarning("Ignoring %s since it requires %d bits to represent"\
                             " when target requires %d",                        \
                             cilog2(llabs(_ival)), 8 * sizeof(dst->_id) -1);    \
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

#define TS_BOOL(_id)                                                            \
do {                                                                            \
    if (!strcmpi(#_id, key)) {                                                  \
        if (!strcmpi(value, "true") || !strcmpi(value, "yes") || !strcmpi(value, "1")) {\
            dst->_id = true;                                                    \
        }                                                                       \
        else if (!strcmpi(value, "false") || !strcmpi(value, "no") || !strcmpi(value, "0")) {\
            dst->_id = false;                                                   \
        }                                                                       \
        else {                                                                  \
            fsLogWarning("Invalid boolean value encountered %s", value);        \
        }                                                                       \
                                                                                \
        return;                                                                 \
    }                                                                           \
} while (0)

#define TS_KEY(_id, _vkey)                                                      \
do {                                                                            \
    if (!strcmpi(#_id, key)) {                                                  \
        fsiAddToKeymap(p, _vkey, value, false);                                 \
        return;                                                                 \
    }                                                                           \
} while (0)

#else

#define fsTryParseIniFile(p, v, fname)

#define TS_INT(_id)
#define TS_INT_RANGE(_id, _lo, _hi)
#define TS_INT_FUNC(_id, _func)
#define TS_BOOL(_id)
#define TS_KEY(_id, _vkey)

#endif // FS_DISABLE_OPTION

#endif // FS_OPTION_H

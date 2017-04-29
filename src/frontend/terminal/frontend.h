///
// frontend.h
// ==========
//
// Header file for the faststack terminal implementation. This is solely done
// to seperate the many compile-time configuration variables and declarations
// from the actual implementation.
///

#include "keymap.h"
#include "glyph.h"
#include <faststack.h>
#include <stdint.h>
#include <termios.h>

///
// Offsets and lengths of the display field.
///
#define FS_TERM_WIDTH  76
#define FS_TERM_HEIGHT 26

// Preview offsets
#define HOLD_X  (2)
#define HOLD_Y  (2)
#define HOLD_H  (3   + 2)
#define HOLD_W  (2*3 + 2)

// Field offsets
#define FIELD_X (HOLD_X + HOLD_W + 1)
#define FIELD_Y HOLD_Y
#define FIELD_H (f->fieldHeight - f->fieldHidden + 1)
#define FIELD_W (2 * f->fieldWidth + 2)

// Preview offsets
#define PVIEW_X (FIELD_X + FIELD_W + 2)
#define PVIEW_Y FIELD_Y
#define PVIEW_H (4 * (1 + 3 + 1))
#define PVIEW_W (2*4 + 2)

// Info offsets
#define INFO_X  (PVIEW_X + PVIEW_W + 2)
#define INFO_Y  PVIEW_Y
#define INFO_H  (-1)                      // Unused
#define INFO_W  (-1)                      // Unused


///
// Attributes used when displaying cells.
#define ATTR_COUNT 13

enum CellAttribute {
    ATTR_REVERSE = 0x0001,
    ATTR_BLACK   = 0x0002,
    ATTR_RED     = 0x0004,
    ATTR_GREEN   = 0x0008,
    ATTR_YELLOW  = 0x0010,
    ATTR_BLUE    = 0x0020,
    ATTR_MAGENTA = 0x0040,
    ATTR_CYAN    = 0x0080,
    ATTR_WHITE   = 0x0100,
    ATTR_UNDERLINE = 0x0200,
    ATTR_BRIGHT  = 0x0400,
    ATTR_DIM     = 0x0800,
    ATTR_BLINK   = 0x1000
};

///
// Represents a single cell of the terminal.
///
typedef struct {
    /// Value stored in this cell.
    uint32_t value;

    /// Attributes associated with this cell.
    uint16_t attrs;
} TerminalCell;

///
// Represents a single entry in a keymap.
typedef struct {
    /// Is this a default key (can be overridden)
    bool isDefault;

    /// The value of the key
    int value;
} KeyEntry;

///
// faststack Platform Specific View
//
// Represents a generic view which is passed around by the faststack interface
// code.
///
struct FSFrontend {
    /// A generic platform-independent view.
    FSView *view;

    /// Mapping from virtual keycode to physical keycode.
    KeyEntry keymap[FST_VK_COUNT][FS_MAX_KEYS_PER_ACTION];

    /// File descriptor of currently open input device.
    int inputFd;

    /// Initial terminal state.
    struct termios initialTerminalState;

    /// Indicate whether complete redraw must occur.
    bool invalidateBuffers;

    /// Terminal width
    int16_t width;

    /// Terminal height
    int16_t height;

    /// Should the field be centered?
    bool centerField;

    /// Glyphs to use when displaying field items.
    GlyphSet glyph;

    /// Field framebuffers.
    //
    // Each buffer stores a utf8 codepoint along with a set of possible
    // attribute modifiers.
    TerminalCell bbuf[FS_TERM_HEIGHT][FS_TERM_WIDTH];
    TerminalCell fbuf[FS_TERM_HEIGHT][FS_TERM_WIDTH];
};

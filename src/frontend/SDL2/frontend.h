///
// frontend.h
// ==========
//
// Header file for the FastStack terminal implementation. This is solely done
// to seperate the many compile-time configuration variables and declarations
// from the actual implementation.
///

#include "keymap.h"
#include "fs.h"
#include "fsInterface.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

// Font and sound effect resources are embedded in the binary itself.
//
// Notes:
//  * Determine if we ever want to dynamically load at any point.
//
#ifdef USE_SOUND
#   include <SDL_mixer.h>
#endif

// Specifies the pt value of the font to load
#define DEFAULT_FONT_SIZE 20

// This should be kept moderately low to ensure that buffered data is not
// delayed too much.
#define AUDIO_BUFFER_SIZE 512

///
// Offsets and lengths of the display field.
///

// The multiplier here should divide the width of the field, else black bars
// will be present.
//
// For reference; 800 * 0.02625 = 21.
#define BLOCK_SL (v->width * 0.02625)

// Field offsets
#define FIELD_X (v->width * 0.13125)
#define FIELD_Y (v->height * 0.15)
#define FIELD_W (v->view->game->fieldWidth * BLOCK_SL)
#define FIELD_H (v->view->game->fieldHeight * BLOCK_SL)

// Hold offsets
#define HOLDP_X (FIELD_X - (4.5f * BLOCK_SL))
#define HOLDP_Y (FIELD_Y)
#define HOLDP_W (BLOCK_SL * 4)
#define HOLDP_H (BLOCK_SL * 4);

// Preview offsets
#define PVIEW_X (FIELD_X + FIELD_W + 10)
#define PVIEW_Y (FIELD_Y)
#define PVIEW_W (BLOCK_SL * 4)
#define PVIEW_H (FIELD_H)

// Info offsets
#define INFOS_X (PVIEW_X + PVIEW_W + 20)
#define INFOS_Y (FIELD_Y)
#define INFOS_W (v->width * 0.125)
#define INFOS_H (FIELD_H)

///
// FastStack Platform Specific View
//
// Represents a generic view which is passed around by the FastStack interface
// code.
///
struct FSPSView {
    // The generic backing view
    FSView *view;

    // Keymap mapping an action to a number of keycodes
    SDL_Keycode keymap[FST_VK_COUNT][FS_MAX_KEYS_PER_ACTION];

    // Platform-specific render structures
    //
    // The window to render to
    SDL_Window *window;

    // The renderer that backs this window
    SDL_Renderer *renderer;

    // We a single font specification while rendering.
    TTF_Font *font;

#ifdef USE_SOUND
    // Sound effect data
    Mix_Chunk *seBuffer[FST_SE_COUNT];
#endif

    // The current width of the window
    int width;

    // The current height of the window
    int height;

    // Did we receive a restart event?
    bool restart;

    // Should we display the debug screen?
    bool showDebug;
};

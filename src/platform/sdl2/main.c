///
// main.c
//
// SDL2 frontend for FastStack.
///

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include <fs.h>
#include <fsInterface.h>

// Not usual, but just includes these definitions directly.
#include "keymap.h"

// We define the font directly in memory since we only want to use one, and it
// reduces the requirement of having seperate resource files.
#include "ProFont.res"
const int FontSize = 20;

// Field positional locations

// Individual block length
// The multiplier here should exactly divide the width of the field else we will
// get black bards between squares.
// 800 * 0.02625 = 21 for reference.
#define BLOCK_SL (v->width * 0.02625)

// Main field bounding rectangle
#define FIELD_X (v->width * 0.13125)
#define FIELD_Y (v->height * 0.15)
#define FIELD_W (v->view->game->fieldWidth * BLOCK_SL)
#define FIELD_H (v->view->game->fieldHeight * BLOCK_SL)

// Preview bounding rectangle
#define HOLDP_X (FIELD_X - (4.5f * BLOCK_SL))
#define HOLDP_Y (FIELD_Y)
#define HOLDP_W (BLOCK_SL * 4)
#define HOLDP_H (BLOCK_SL * 4);

// Preview area bounding rectangle
#define PVIEW_X (FIELD_X + FIELD_W + 10)
#define PVIEW_Y (FIELD_Y)
#define PVIEW_W (BLOCK_SL * 4)
#define PVIEW_H (FIELD_H)

// Info section bounding rectangle
#define INFOS_X (PVIEW_X + PVIEW_W + 20)
#define INFOS_Y (FIELD_Y)
#define INFOS_W (v->width * 0.125)
#define INFOS_H (FIELD_H)

// A platform-specific view.
struct FSPSView {
    // The generic backing view
    FSView *view;

    // Keymap mapping an action to a number of keycodes
    SDL_Keycode keymap[VKEY_COUNT][FS_MAX_KEYS_PER_ACTION];

    // Platform-specific render structures
    //
    // The window to render to
    SDL_Window *window;

    // The renderer that backs this window
    SDL_Renderer *renderer;

    // We a single font specification while rendering.
    TTF_Font *font;

    // The current width of the window
    int width;

    // The current height of the window
    int height;

    // Should we display the debug screen?
    bool showDebug;
};

void initSDL(FSPSView *v)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fsLogFatal("SDL_Init error: %s", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() == -1) {
        fsLogFatal("TTF_Init error: %s", TTF_GetError());
        SDL_Quit();
        exit(1);
    }

    // Load the font defined in Unibody8.font
    SDL_RWops *rw = SDL_RWFromConstMem(ttfFontSpec, ttfFontSpecLen);
    v->font = TTF_OpenFontRW(rw, 1, FontSize);
    if (v->font == NULL) {
        fsLogFatal("TTF_OpenFontIndexRW error: %s", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    v->width = 800;
    v->height = 600;
    v->showDebug = false;

    if (SDL_CreateWindowAndRenderer(v->width, v->height, SDL_WINDOW_SHOWN,
                                    &v->window, &v->renderer)) {
        fsLogFatal("SDL_CreateWindowAndRenderer error: %s", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    SDL_SetWindowTitle(v->window, "FastStack");

    // Clear the entire screen here, since each draw function only manages the
    // section of the screen it can draw to.
    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderClear(v->renderer);
}

void destroySDL(FSPSView *v)
{
    SDL_DestroyRenderer(v->renderer);
    SDL_DestroyWindow(v->window);
    TTF_CloseFont(v->font);
    TTF_Init();
    SDL_Quit();
}

// We used fixed colours for the moment
const int CRED[7]   = {  5, 238, 249,   7,  93, 250, 237};
const int CGREEN[7] = {186,  23, 187,  94, 224, 105, 225};
const int CBLUE[7]  = {221, 234,   0, 240,  31,   0,   0};

// Defines how we work out a color pattern for a specific block id.
#define BLOCK_RGBA_TRIPLE(id) CRED[(id)], CGREEN[(id)], CBLUE[(id)], 255

FSLong fsiGetTime(FSPSView *v)
{
    (void) v;
    return SDL_GetTicks() * 1000;
}

// Sleep for the specified number of microseconds.
void fsiSleepUs(FSPSView *v, FSLong time)
{
    (void) v;
    SDL_Delay(time / 1000);
}

// Handle window resize
void handleWindowEvents(FSPSView *v, const Uint8 *state)
{
    // Handle quit event
    if (state[SDL_GetScancodeFromKey(SDLK_q)] || SDL_QuitRequested()) {
        // use a cancel flag
        v->view->game->state = FSS_QUIT;
    }

    // We are only interested in the single press
    static bool uWasJustPressed = false;

    if (state[SDL_GetScancodeFromKey(SDLK_u)] && !uWasJustPressed) {
        uWasJustPressed = true;
        v->showDebug = !v->showDebug;
    }
    else if (!state[SDL_GetScancodeFromKey(SDLK_u)]) {
        uWasJustPressed = false;
    }

    // Hint that we need to clear the screen
}

// Return the set of virtual keys that were read from the physical device.
FSBits fsiReadKeys(FSPSView *v)
{
    (void) v;

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // Check through all values in the keymap
    FSBits keys = 0;
    for (int i = 0; i < VKEY_COUNT; ++i) {
        // Check just the first keycode for now (will extend)
        if (state[SDL_GetScancodeFromKey(v->keymap[i][0])]) {
            // Equivalent to mapping VKEYI -> VKEY
            keys |= (1 << i);
        }
    }

    return keys;
}

// We only want to update new events once per frame, as we may read state
// multiple times, so we do this in a pre frame hook.
void fsiPreFrameHook(FSPSView *v)
{
    (void) v;
    SDL_PumpEvents();

    // We handle these events potentially when the game isn't running
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    handleWindowEvents(v, state);
}

// Render the string to the specified coordinates
//
// Consider improving this to avoid software rendering.
// We perform quite a few copies as every string render requires a new
// surface to be created and destroyed.
//
// Should ideally pre-map each font to a texture and copy regions to the
// text of the pre-rendered glyphs. Would take a bit of work, so will not
// do unless it becomes a noticeable bottleneck/problem.
inline static void renderString(FSPSView *v, const char *s, int x, int y)
{
    SDL_Color color = { BLOCK_RGBA_TRIPLE(4) };
    SDL_Surface *textSurface = TTF_RenderText_Solid(v->font, s, color);
    if (textSurface == NULL) {
        fsLogFatal("TTF_RenderText_Solid error: %s", TTF_GetError());
        destroySDL(v);
        exit(1);
    }

    // Copy a texture to the current renderer
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(v->renderer, textSurface);
    if (textTexture == NULL) {
        fsLogFatal("SDL_CreateTextureFromSurface error: %s", TTF_GetError());
        SDL_FreeSurface(textSurface);
        destroySDL(v);
        exit(1);
    }

    // Size the input method correctly
    int w, h;
    if (TTF_SizeText(v->font, s, &w, &h) == -1) {
        fsLogFatal("TTF_SizeText error: %s", TTF_GetError());
        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);
        destroySDL(v);
        exit(1);
    }

    // We limit the debug space to the rightmost 20%. If the data is too long,
    // it is simply truncated.
    const SDL_Rect dst = {
        .x = x, .y = y,
        .w = w, .h = h
    };

    SDL_RenderCopy(v->renderer, textTexture, NULL, &dst);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

// Called at the end of every frame
void fsiPostFrameHook(FSPSView *v)
{
    (void) v;
}

void drawDebug(FSPSView *v)
{
    const FSGame *f = v->view->game;

    // Initial x, y offset
    int ux = v->width * 0.7;
    int uy = 1;

    // Clear the debug area, this extends to the edges of the screen
    const SDL_Rect area = {
        .x = ux, .y = uy,
        .w = v->width,
        .h = v->height
    };

    // Clear the entire screen space before continuing
    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(v->renderer, &area);


    // Print the game logic fps and the render fps.
    // assume non-zero for now (maybe not if we had a supercomputer).
    const int elapsedTime = fsiGetTime(v);

    // Should only look at a window here  but oh well
    const float renderFPS = (float) elapsedTime / (1000 * v->view->totalFramesDrawn);
    const float logicFPS = (float) elapsedTime / (1000 * f->totalTicks);

    const int lineSkipY = TTF_FontLineSkip(v->font);
    const int writeBufferSize = 64;
    char writeBuffer[writeBufferSize];

    // Which Y position we are currently drawing to
    int c = 0;

    snprintf(writeBuffer, writeBufferSize, "Render FPS: %.5f", renderFPS);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "Logic FPS: %.5f", logicFPS);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    // Render current block position
    snprintf(writeBuffer, writeBufferSize, "Block:");
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "          x: %d", f->x);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "          y: %d", f->y);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "      theta: %d", f->theta);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "  hardDropY: %d", f->hardDropY);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "Field:");
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    /*
    snprintf(writeBuffer, writeBufferSize, "      state: %s", fsStateToStr(f->state));
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);
    */

    snprintf(writeBuffer, writeBufferSize, "    gravity: %.3f", f->gravity);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "Input:");
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "   rotation: %d", f->lastInput.rotation);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "   movement: %d", f->lastInput.movement);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "    gravity: %d", f->lastInput.gravity);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "      extra: %d", f->lastInput.extra);
    renderString(v, writeBuffer, ux, uy + c++ * lineSkipY);
}

// Draw the hold piece
static void drawHoldPiece(FSPSView *v)
{
    SDL_Rect block = {
        .x = -1,
        .y = -1,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    const FSBlock pid = v->view->game->holdPiece;
    if (pid == FS_NONE)
        return;

    FSInt2 blocks[4];
    fsPieceToBlocks(v->view->game, blocks, pid, 0, 0, 0);
    SDL_SetRenderDrawColor(v->renderer, BLOCK_RGBA_TRIPLE(pid));

    int bxoff = pid != 3 && pid != 0 ? BLOCK_SL / 2 : 0;
    int byoff = pid == 0 ? BLOCK_SL / 2 : 0;

    for (int i = 0; i < 4; ++i) {
        block.x = bxoff + HOLDP_X + blocks[i].x * BLOCK_SL;
        block.y = byoff + HOLDP_Y + blocks[i].y * BLOCK_SL;

        SDL_RenderFillRect(v->renderer, &block);
    }
}

// Draw piece and its shadow to the field.
//
// This must be called AFTER drawField, since that will overwrite these
// render calls.
static void drawPieceAndShadow(FSPSView *v)
{
    // We need to draw the actual piece last in case there is overlap
    SDL_Rect block = {
        .x = -1,
        .y = -1,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    const FSBlock pid = v->view->game->piece;

    // We have no piece to draw
    if (pid == FS_NONE)
        return;

    FSInt2 blocks[4];
    fsPieceToBlocks(v->view->game, blocks, pid, v->view->game->x,
            v->view->game->hardDropY, v->view->game->theta);

    for (int i = 0; i < 4; ++i) {
        block.x = FIELD_X + blocks[i].x * BLOCK_SL;
        block.y = FIELD_Y + blocks[i].y * BLOCK_SL;

        // Shade colourd down
        SDL_SetRenderDrawColor(v->renderer,
                CRED[pid] / 2,
                CGREEN[pid] / 2,
                CBLUE[pid] / 2,
                255);
        SDL_RenderFillRect(v->renderer, &block);
    }

    fsPieceToBlocks(v->view->game, blocks, pid, v->view->game->x,
            v->view->game->y, v->view->game->theta);

    for (int i = 0; i < 4; ++i) {
        block.x = FIELD_X + blocks[i].x * BLOCK_SL;
        block.y = FIELD_Y + blocks[i].y * BLOCK_SL;

        SDL_SetRenderDrawColor(v->renderer, BLOCK_RGBA_TRIPLE(pid));
        SDL_RenderFillRect(v->renderer, &block);
    }
}

// Draw the field, current piece and shadow.
//
// We render onto a 4:3 aspect ratio occupying the following axial space:
//  x - [10%, 35.63%]
//  y - [15%, 83.333%]
//
//  We assume a width and height of 10, 20 for the moment.
void drawField(FSPSView *v)
{
    const SDL_Rect border = {
        .x = FIELD_X - 1,
        .y = FIELD_Y - 1,
        .w = FIELD_W + 2,
        .h = FIELD_H + 2
    };

    // Clear the entire screen space before continuing
    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(v->renderer, &border);

    SDL_SetRenderDrawColor(v->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(v->renderer, &border);

    SDL_Rect block = {
        .x = 0,
        .y = 0,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    for (int y = 0; y < v->view->game->fieldHeight; ++y){
        block.y = FIELD_Y + y * BLOCK_SL;
        for (int x = 0; x < v->view->game->fieldWidth; ++x) {
            block.x = FIELD_X + x * BLOCK_SL;
            if (v->view->game->b[y][x] > 0) {
                // Grey colour
                SDL_SetRenderDrawColor(v->renderer, 140, 140, 140, 255);
                SDL_RenderFillRect(v->renderer, &block);
            }
            else {
                SDL_SetRenderDrawColor(v->renderer, BLOCK_RGBA_TRIPLE(1));
                SDL_RenderDrawRect(v->renderer, &block);
            }
        }
    }
}

static void drawPreviewSection(FSPSView *v)
{
    // Clear preview area
    const SDL_Rect border = {
        .x = PVIEW_X,
        .y = PVIEW_Y,
        .w = PVIEW_W,
        .h = PVIEW_H
    };

    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(v->renderer, &border);

    SDL_Rect block = {
        .x = PVIEW_X,
        .y = 0,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    // Print 4 preview pieces for now
    for (int i = 0; i < 4; ++i) {
        FSInt2 blocks[4];
        const FSBlock pid = v->view->game->nextPiece[i];
        fsPieceToBlocks(v->view->game, blocks, pid, 0, 0, 0);

        // Set field to grey currently
        const int by = PVIEW_Y + BLOCK_SL * (i * 4);
        SDL_SetRenderDrawColor(v->renderer, BLOCK_RGBA_TRIPLE(pid));

        for (int j = 0; j < 4; ++j) {
            block.y = by + BLOCK_SL * blocks[j].y;
            block.x = PVIEW_X + BLOCK_SL * blocks[j].x;

            // I-piece
            if (v->view->game->nextPiece[i] == 0) {
                block.y += BLOCK_SL / 2;
            }
            // Non O-Pieces
            else if (v->view->game->nextPiece[i] != 3) {
                block.x += BLOCK_SL / 2;
            }

            SDL_RenderFillRect(v->renderer, &block);
        }
    }
}

static void drawInfoSection(FSPSView *v)
{
    // Render text to bottom of the screen signalling goal
    const int writeBufferSize = 64;
    char writeBuffer[writeBufferSize];

    int remaining = v->view->game->goal - v->view->game->linesCleared;
    if (remaining < 0)
        remaining = 0;

    snprintf(writeBuffer, writeBufferSize, "%d", remaining);
    renderString(v, writeBuffer, FIELD_X + FIELD_W / 2 - 10, FIELD_Y + FIELD_H + BLOCK_SL);

    // Clear the screen on the segment we draw info
    const SDL_Rect infoSegment = {
        .x = INFOS_X,
        .y = INFOS_Y,
        .w = INFOS_W,
        .h = INFOS_H
    };

    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(v->renderer, &infoSegment);

    SDL_SetRenderDrawColor(v->renderer, BLOCK_RGBA_TRIPLE(1));

    // Draw right side info
    const int lineSkipY = TTF_FontLineSkip(v->font);
    int c = 0;

    snprintf(writeBuffer, writeBufferSize, "Time");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    const int msElapsed = v->view->game->msPerTick * v->view->game->totalTicks;
    snprintf(writeBuffer, writeBufferSize, "%.3f", (float) msElapsed / 1000);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Blocks Placed");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "%d", v->view->game->blocksPlaced);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Tetriminos Per Second (TPS)");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "%.5f",
            (float) v->view->game->blocksPlaced / ((float) msElapsed / 1000));
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Finesse");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);
    snprintf(writeBuffer, writeBufferSize, "%d", v->view->game->finesse);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);
}

// Draw the prewview pieces
void fsiDraw(FSPSView *v)
{
    // Clear entire screen
    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderClear(v->renderer);

    drawField(v);
    drawHoldPiece(v);
    drawPieceAndShadow(v);
    drawPreviewSection(v);
    drawInfoSection(v);

    if (v->showDebug)
        drawDebug(v);
}

void fsiBlit(FSPSView *v)
{
    SDL_RenderPresent(v->renderer);
}

void fsiAddToKeymap(FSPSView *v, int virtualKey, const char *keyValue)
{
    const SDL_Keycode kc = fsKeyToPhysicalKey(keyValue);
    if (kc) {
        v->keymap[virtualKey][0] = kc;
    }
}

// parse ini will call this function and pass appropriate values
void fsiUnpackFrontendOption(FSPSView *v, const char *key, const char *value)
{
    (void) value;

    if (!strcmpi(key, "debug")) {
        v->showDebug = false;
    }
}

// Should probably provide a menu and ability to play another game/restart.
// Replays and other things should be an option specified.
int main(void)
{
    fsCurrentLogLevel = FS_LOG_LEVEL_DEBUG;

    FSGame game;
    FSControl control;
    // Generic View
    FSView gView = { .game = &game, .control = &control, .totalFramesDrawn = 0 };
    // Platform-Specific View
    FSPSView pView = { .view = &gView };

    initSDL(&pView);

    fsGameClear(&game);
    fsParseIniFile(&pView, &gView, FS_CONFIG_FILENAME);
    fsGameLoop(&pView, &gView);

    // Give time to read final scores (temporary)
    if (gView.game->state == FSS_GAMEOVER)
        SDL_Delay(3000);

    destroySDL(&pView);
}

///
// frontend.c
// ==========
//
// SDL2 frontend for FastStack.
///

#include "frontend.h"

#include "font.inc"

#ifdef USE_SOUND
#   include "sound.inc"
#endif

const char *fsiFrontendName = "sdl2";

void fsiPreInit(FSFrontend *v)
{
    // These defaults can be overridden by an ini file.
    v->width = 800;
    v->height = 600;
    v->showDebug = false;

    // Initial keybinds are the only reason we need a pre-initialization phase.
    for (int i = 0; i < FST_VK_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            v->keymap[i][j] = (KeyEntry) {
                .isDefault = false,
                .value = KEY_NONE
            };
        }
    }
}

int calcFontSize(int width)
{
    return width / 40;
}

void fsiInit(FSFrontend *v)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fsLogFatal("SDL_Init error: %s", SDL_GetError());
        exit(1);
    }

    // TODO: Create the window after we have processed options if possible
    // to allow for dynamically specified values. Should still match the
    // given aspect ratio however.
    if (SDL_CreateWindowAndRenderer(v->width, v->height, SDL_WINDOW_SHOWN,
                                    &v->window, &v->renderer)) {
        fsLogFatal("SDL_CreateWindowAndRenderer error: %s", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    SDL_RWops *rw = SDL_RWFromConstMem(ttfFontSpec, ttfFontSpecLen);
    v->font = FC_CreateFont();
    FC_LoadFont_RW(v->font, v->renderer, rw, 1, calcFontSize(v->width),
                   FC_MakeColor(200, 200, 200, 255), TTF_STYLE_NORMAL);

#ifdef USE_SOUND
    if (Mix_OpenAudio(22050, AUDIO_S16LSB, 1, AUDIO_BUFFER_SIZE) < 0) {
        fsLogFatal("Mix_OpenAudio error: %s", Mix_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(v->renderer);
        SDL_DestroyWindow(v->window);
        SDL_Quit();
        exit(1);
    }

    // TODO: Find a cleaner way of performing this if possible.
    #define LoadWav(seName, enumName)                                   \
    do {                                                                \
        if (!(v->seBuffer[FST_SE_##enumName] = Mix_QuickLoad_WAV(seName##_wav))) {\
            fsLogFatal("Mix_QuickLoadWAV error: %s", Mix_GetError());     \
            SDL_DestroyRenderer(v->renderer);                           \
            SDL_DestroyWindow(v->window);                               \
            SDL_Quit();                                                 \
            exit(1);                                                    \
        }                                                               \
    } while(0)

    LoadWav(gameover, GAMEOVER);
    LoadWav(ready, READY);
    LoadWav(go, GO);
    LoadWav(piece0, IPIECE);
    LoadWav(piece1, JPIECE);
    LoadWav(piece2, LPIECE);
    LoadWav(piece3, OPIECE);
    LoadWav(piece4, SPIECE);
    LoadWav(piece5, TPIECE);
    LoadWav(piece6, ZPIECE);
    LoadWav(move, MOVE);
    LoadWav(rotate, ROTATE);
    LoadWav(hold, HOLD);
    LoadWav(erase1, ERASE1);
    LoadWav(erase2, ERASE2);
    LoadWav(erase3, ERASE3);
    LoadWav(erase4, ERASE4);

    #undef LoadWav
#endif

    SDL_SetWindowTitle(v->window, "FastStack");
    SDL_SetRenderDrawColor(v->renderer, 0, 0, 0, 255);
    SDL_RenderClear(v->renderer);
}

void fsiFini(FSFrontend *v)
{
#ifdef USE_SOUND
    // Assume WAV data is reclaimed by the OS for now
    Mix_CloseAudio();
#endif

    FC_FreeFont(v->font);
    SDL_DestroyRenderer(v->renderer);
    SDL_DestroyWindow(v->window);
    SDL_Quit();
}

// We used fixed colours for the moment
const int CRED[7]   = {  5, 238, 249,   7,  93, 250, 237};
const int CGREEN[7] = {186,  23, 187,  94, 224, 105, 225};
const int CBLUE[7]  = {221, 234,   0, 240,  31,   0,   0};

// Defines how we work out a color pattern for a specific block id.
#define BLOCK_RGBA_TRIPLE(id) CRED[(id)], CGREEN[(id)], CBLUE[(id)], 255

i32 fsiGetTime(FSFrontend *v)
{
    (void) v;
    return SDL_GetTicks() * 1000;
}

void fsiSleep(FSFrontend *v, i32 time)
{
    (void) v;
    SDL_Delay(time / 1000);
}

u32 fsiReadKeys(FSFrontend *v)
{
    (void) v;
    SDL_PumpEvents();
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    u32 keys = 0;
    for (int i = 0; i < FST_VK_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            if (v->keymap[i][j].value == KEY_NONE) {
                break;
            }

            if (state[SDL_GetScancodeFromKey(v->keymap[i][j].value)]) {
                keys |= FS_TO_FLAG(i);
            }
        }
    }

    return keys;
}

// Audio is not buffered by default under SDL_Mixer which is what we want.
void fsiPlaySe(FSFrontend *v, u32 se)
{
#ifdef USE_SOUND
    #define PlayWav(name)                                                   \
    do {                                                                    \
        if (se & FST_SE_FLAG_##name) {                                      \
            if (Mix_PlayChannel(-1, v->seBuffer[FST_SE_##name], 0) == -1) { \
                fsLogWarning("Mix_PlayChannel error: %s", Mix_GetError());  \
            }                                                               \
        }                                                                   \
    } while (0)

    PlayWav(MOVE);
    PlayWav(GAMEOVER);
    PlayWav(READY);
    PlayWav(GO);
    PlayWav(IPIECE);
    PlayWav(JPIECE);
    PlayWav(LPIECE);
    PlayWav(OPIECE);
    PlayWav(SPIECE);
    PlayWav(TPIECE);
    PlayWav(ZPIECE);
    PlayWav(MOVE);
    PlayWav(ROTATE);
    PlayWav(HOLD);
    PlayWav(ERASE1);
    PlayWav(ERASE2);
    PlayWav(ERASE3);
    PlayWav(ERASE4);

    #undef PlayWav
#else
    (void) v;
    (void) se;
#endif
}

// Render the string to the specified coordinates
//
// Notes:
//  * This is really unoptimized. We should create a surface mapping on font
//    load and simply reuse this for every character instead.
static void renderString(FSFrontend *v, const char *s, int x, int y)
{
    FC_Draw(v->font, v->renderer, x, y, s);
}

///
// Render a string onto the middle of the field.
//
// The string will be centered, and truncated if too long.
void fsiRenderFieldString(FSFrontend *v, const char *msg)
{
    int w = FC_GetWidth(v->font, msg);
    renderString(v, msg, FIELD_X + FIELD_W / 2 - w / 2 , FIELD_Y + FIELD_H / 2);
}

///
// We only need to pump events once per frame.
void fsiPreFrameHook(FSFrontend *v)
{
    SDL_PumpEvents();

    // A raw window quit just exits without cleaning up
    if (SDL_QuitRequested()) {
        exit(0);
    }

    // Debug window with release catch
    static bool justDown = false;
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_GetScancodeFromKey(SDLK_u)] && !justDown) {
        justDown = true;
        v->showDebug = !v->showDebug;
    }
    else if (!state[SDL_GetScancodeFromKey(SDLK_u)]) {
        justDown = false;
    }
}

///
// Execute after everything else in the frame.
void fsiPostFrameHook(FSFrontend *v)
{
    (void) v;
}

///
// Draws a rudimentary debug screen in the upper right corner of the screen.
void drawDebug(FSFrontend *v)
{
    const FSEngine *f = v->view->game;

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
    const float renderFPS = (float) elapsedTime / (1000 * f->totalTicks / f->ticksPerDraw);
    const float logicFPS = (float) elapsedTime / (1000 * f->totalTicks);

    const int lineSkipY = FC_GetLineHeight(v->font);
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

    snprintf(writeBuffer, writeBufferSize, "    gravity: %.3f", (float) f->gravity / 1000000);
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

static void drawHoldPiece(FSFrontend *v)
{
    SDL_Rect block = {
        .x = -1,
        .y = -1,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    const FSEngine *f = v->view->game;

    if (f->holdPiece == FS_NONE) {
        return;
    }

    i8x2 blocks[FS_NBP];
    fsGetBlocks(f, blocks, f->holdPiece, 0, 0, 0);
    SDL_SetRenderDrawColor(v->renderer, BLOCK_RGBA_TRIPLE(f->holdPiece));

    int bxoff = f->holdPiece != FS_O && f->holdPiece != 0 ? BLOCK_SL / 2 : 0;
    int byoff = f->holdPiece == FS_I ? BLOCK_SL / 2 : 0;

    for (int i = 0; i < FS_NBP; ++i) {
        block.x = bxoff + HOLDP_X + blocks[i].x * BLOCK_SL;
        block.y = byoff + HOLDP_Y + blocks[i].y * BLOCK_SL;

        SDL_RenderFillRect(v->renderer, &block);
    }
}

///
// This **must** be called after drawField to ensure the piece and shadow
// is above any lying pieces.
static void drawPieceAndShadow(FSFrontend *v)
{
    // We need to draw the actual piece last in case there is overlap
    SDL_Rect block = {
        .x = -1,
        .y = -1,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    const FSEngine *f = v->view->game;
    const FSBlock pid = f->piece;

    if (pid == FS_NONE) {
        return;
    }

    i8x2 blocks[FS_NBP];
    fsGetBlocks(f, blocks, pid, f->x, f->hardDropY - f->fieldHidden, f->theta);

    for (int i = 0; i < FS_NBP; ++i) {
        block.x = FIELD_X + blocks[i].x * BLOCK_SL;
        block.y = FIELD_Y + blocks[i].y * BLOCK_SL;

        // Filter blocks greater than visible field height
        if (blocks[i].y < 0) {
            continue;
        }

        // Dim ghost colour to be less focused
        SDL_SetRenderDrawColor(v->renderer,
                CRED[pid] / 2,
                CGREEN[pid] / 2,
                CBLUE[pid] / 2,
                255);
        SDL_RenderFillRect(v->renderer, &block);
    }

    fsGetBlocks(f, blocks, pid, f->x, f->y - f->fieldHidden, f->theta);

    for (int i = 0; i < FS_NBP; ++i) {
        block.x = FIELD_X + blocks[i].x * BLOCK_SL;
        block.y = FIELD_Y + blocks[i].y * BLOCK_SL;

        // Filter blocks greater than visible field height
        if (blocks[i].y < 0) {
            continue;
        }

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
void drawField(FSFrontend *v)
{
    const FSEngine *f = v->view->game;

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

    for (int y = f->fieldHidden; y < f->fieldHeight; ++y){
        block.y = FIELD_Y + (y - f->fieldHidden) * BLOCK_SL;
        for (int x = 0; x < f->fieldWidth; ++x) {
            block.x = FIELD_X + x * BLOCK_SL;
            if (f->b[y][x] > 0) {
                // Grey colour
                SDL_SetRenderDrawColor(v->renderer, 140, 140, 140, 255);
                SDL_RenderFillRect(v->renderer, &block);
            }
        }
    }
}

static void drawPreviewSection(FSFrontend *v)
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

    const FSEngine *f = v->view->game;
    const int previewCount = f->nextPieceCount > FS_MAX_PREVIEW_COUNT
                                ? FS_MAX_PREVIEW_COUNT
                                : f->nextPieceCount;

    // Print 4 preview pieces max for now (where do we render if higher?)
    for (int i = 0; i < previewCount; ++i) {
        i8x2 blocks[FS_NBP];
        const FSBlock pid = f->nextPiece[i];
        fsGetBlocks(f, blocks, pid, 0, 0, 0);

        // Set field to grey currently
        const int by = PVIEW_Y + BLOCK_SL * (i * 4);
        SDL_SetRenderDrawColor(v->renderer, BLOCK_RGBA_TRIPLE(pid));

        for (int j = 0; j < FS_NBP; ++j) {
            block.y = by + BLOCK_SL * blocks[j].y;
            block.x = PVIEW_X + BLOCK_SL * blocks[j].x;

            // These offsets are only valid if the entryTheta is standard.
            // Sega entryTheta's have incorrect spacing still, but we'll
            // consider this okay since it is less used.
            if (f->nextPiece[i] == FS_I) {
                block.y -= BLOCK_SL / 2;
            }
            else if (f->nextPiece[i] != FS_O) {
                block.x += BLOCK_SL / 2;
            }

            SDL_RenderFillRect(v->renderer, &block);
        }
    }
}

static void drawInfoSection(FSFrontend *v)
{
    const FSEngine *f = v->view->game;

    // Render text to bottom of the screen signalling goal
    const int writeBufferSize = 64;
    char writeBuffer[writeBufferSize];

    int remaining = f->goal - f->linesCleared;
    if (remaining < 0) {
        remaining = 0;
    }

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
    const int lineSkipY = FC_GetLineHeight(v->font);
    int c = 0;

    snprintf(writeBuffer, writeBufferSize, "Time");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    const int msElapsed = f->msPerTick * f->totalTicks;
    snprintf(writeBuffer, writeBufferSize, "%.3f", (float) msElapsed / 1000);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Blocks");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "%d", f->blocksPlaced);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "TPS");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "%.5f",
            msElapsed != 0
             ? (float) f->blocksPlaced / ((float) msElapsed / 1000)
             : 0);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "KPT");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);
    snprintf(writeBuffer, writeBufferSize, "%.5f",
             f->blocksPlaced ? (float) f->totalKeysPressed / f->blocksPlaced
                             : 0.0f
    );
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Faults");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);
    snprintf(writeBuffer, writeBufferSize, "%d", f->finesse);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);
}

// Draw the prewview pieces
void fsiDraw(FSFrontend *v)
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

void fsiBlit(FSFrontend *v)
{
    SDL_RenderPresent(v->renderer);
}

void fsiAddToKeymap(FSFrontend *v, int virtualKey, const char *keyValue, bool isDefault)
{
    const SDL_Keycode kc = fsKeyToPhysicalKey(keyValue);
    if (kc) {
        for (int i = 0; i < FS_MAX_KEYS_PER_ACTION; ++i) {
            KeyEntry *vk = &v->keymap[virtualKey][i];
            if (vk->value == KEY_NONE || vk->isDefault) {
                *vk = (KeyEntry) {
                    .value = kc,
                    .isDefault = isDefault
                };
                return;
            }
        }

        // Keymap was full, warn user
        fsLogWarning("Could not insert key %s into full keymap", keyValue);
    }
}

// parse ini will call this function and pass appropriate values
void fsiUnpackFrontendOption(FSFrontend *v, const char *key, const char *value)
{
    FSFrontend *dst = v;

    TS_BOOL(showDebug);
    TS_INT(height);
    TS_INT(width);

    fsLogWarning("No suitable key found for option %s = %s", key, value);
}

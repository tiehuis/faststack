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

void fsiInit(FSPSView *v)
{
    SDL_RWops *rw;

    v->width = 800;
    v->height = 600;
    v->showDebug = false;
    v->restart = false;

    for (int i = 0; i < FST_VK_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            v->keymap[i][j] = KEY_NONE;
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fsLogFatal("SDL_Init error: %s", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() == -1) {
        fsLogFatal("TTF_Init error: %s", TTF_GetError());
        SDL_Quit();
        exit(1);
    }

    rw = SDL_RWFromConstMem(ttfFontSpec, ttfFontSpecLen);
    v->font = TTF_OpenFontRW(rw, 1, DEFAULT_FONT_SIZE);
    if (v->font == NULL) {
        fsLogFatal("TTF_OpenFontIndexRW error: %s", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    // TODO: Create the window after we have processed options if possible
    // to allow for dynamically specified values. Should still match the
    // given aspect ratio however.
    if (SDL_CreateWindowAndRenderer(v->width, v->height, SDL_WINDOW_SHOWN,
                                    &v->window, &v->renderer)) {
        fsLogFatal("SDL_CreateWindowAndRenderer error: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

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

void fsiFree(FSPSView *v)
{
#ifdef USE_SOUND
    // Assume WAV data is reclaimed by the OS for now
    Mix_CloseAudio();
#endif

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

void fsiSleepUs(FSPSView *v, FSLong time)
{
    (void) v;
    SDL_Delay(time / 1000);
}

FSBits fsiReadKeys(FSPSView *v)
{
    (void) v;
    SDL_PumpEvents();
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    FSBits keys = 0;
    for (int i = 0; i < FST_VK_COUNT; ++i) {
        for (int j = 0; j < FS_MAX_KEYS_PER_ACTION; ++j) {
            if (v->keymap[i][j] == KEY_NONE) {
                break;
            }

            if (state[SDL_GetScancodeFromKey(v->keymap[i][j])]) {
                keys |= FS_TO_FLAG(i);
            }
        }
    }

    return keys;
}

// Audio is not buffered by default under SDL_Mixer which is what we want.
void fsiPlaySe(FSPSView *v, FSBits se)
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
static void renderString(FSPSView *v, const char *s, int x, int y)
{
    SDL_Color color = { BLOCK_RGBA_TRIPLE(4) };
    SDL_Surface *textSurface = TTF_RenderText_Solid(v->font, s, color);
    if (textSurface == NULL) {
        fsLogFatal("TTF_RenderText_Solid error: %s", TTF_GetError());
        fsiFree(v);
        exit(1);
    }

    // Copy a texture to the current renderer
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(v->renderer, textSurface);
    if (textTexture == NULL) {
        fsLogFatal("SDL_CreateTextureFromSurface error: %s", TTF_GetError());
        SDL_FreeSurface(textSurface);
        fsiFree(v);
        exit(1);
    }

    int w, h;
    if (TTF_SizeText(v->font, s, &w, &h) == -1) {
        fsLogFatal("TTF_SizeText error: %s", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    // We limit the debug space to the rightmost 20%, truncating if needed.
    const SDL_Rect dst = {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };

    SDL_RenderCopy(v->renderer, textTexture, NULL, &dst);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

///
// Render a string onto the middle of the field.
//
// The string will be centered, and truncated if too long.
void fsiRenderFieldString(FSPSView *v, const char *msg)
{
    int w, h;
    if (TTF_SizeText(v->font, msg, &w, &h) == -1) {
        fsLogFatal("TTF_SizeText error: %s", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    renderString(v, msg, FIELD_X + FIELD_W / 2 - w / 2 , FIELD_Y + FIELD_H / 2);
}

///
// We only need to pump events once per frame.
void fsiPreFrameHook(FSPSView *v)
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
void fsiPostFrameHook(FSPSView *v)
{
    (void) v;
}

///
// Draws a rudimentary debug screen in the upper right corner of the screen.
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

static void drawHoldPiece(FSPSView *v)
{
    SDL_Rect block = {
        .x = -1,
        .y = -1,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    const FSGame *f = v->view->game;

    if (f->holdPiece == FS_NONE) {
        return;
    }

    FSInt2 blocks[FS_NBP];
    fsPieceToBlocks(f, blocks, f->holdPiece, 0, 0, 0);
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
static void drawPieceAndShadow(FSPSView *v)
{
    // We need to draw the actual piece last in case there is overlap
    SDL_Rect block = {
        .x = -1,
        .y = -1,
        .w = BLOCK_SL,
        .h = BLOCK_SL
    };

    const FSGame *f = v->view->game;
    const FSBlock pid = f->piece;

    if (pid == FS_NONE) {
        return;
    }

    FSInt2 blocks[FS_NBP];
    fsPieceToBlocks(f, blocks, pid, f->x, f->hardDropY, f->theta);

    for (int i = 0; i < FS_NBP; ++i) {
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

    fsPieceToBlocks(f, blocks, pid, f->x, f->y, f->theta);

    for (int i = 0; i < FS_NBP; ++i) {
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
    const FSGame *f = v->view->game;

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

    for (int y = 0; y < f->fieldHeight; ++y){
        block.y = FIELD_Y + y * BLOCK_SL;
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

    const FSGame *f = v->view->game;
    const int previewCount = f->nextPieceCount > 4 ? 4 : f->nextPieceCount;

    // Print 4 preview pieces max for now (where do we render if higher?)
    for (int i = 0; i < previewCount; ++i) {
        FSInt2 blocks[FS_NBP];
        const FSBlock pid = f->nextPiece[i];
        fsPieceToBlocks(f, blocks, pid, 0, 0, 0);

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

static void drawInfoSection(FSPSView *v)
{
    const FSGame *f = v->view->game;

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
    const int lineSkipY = TTF_FontLineSkip(v->font);
    int c = 0;

    snprintf(writeBuffer, writeBufferSize, "Time");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    const int msElapsed = f->msPerTick * f->totalTicks;
    snprintf(writeBuffer, writeBufferSize, "%.3f", (float) msElapsed / 1000);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Blocks Placed");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "%d", f->blocksPlaced);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Tetriminos Per Second (TPS)");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    snprintf(writeBuffer, writeBufferSize, "%.5f",
            msElapsed != 0
             ? (float) f->blocksPlaced / ((float) msElapsed / 1000)
             : 0);
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);

    ++c;

    snprintf(writeBuffer, writeBufferSize, "Finesse");
    renderString(v, writeBuffer, INFOS_X, INFOS_Y + c++ * lineSkipY);
    snprintf(writeBuffer, writeBufferSize, "%d", f->finesse);
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
        for (int i = 0; i < FS_MAX_KEYS_PER_ACTION; ++i) {
            // Found an empty slot to fill
            if (v->keymap[virtualKey][i] == -1) {
                v->keymap[virtualKey][i] = kc;
                return;
            }
        }

        // Keymap was full, warn user
        fsLogWarning("Could not insert key %s into full keymap", keyValue);
    }
}

// parse ini will call this function and pass appropriate values
void fsiUnpackFrontendOption(FSPSView *v, const char *key, const char *value)
{
    FSPSView *dst = v;

    TS_BOOL(showDebug);

    fsLogWarning("No suitable key found for option %s = %s", key, value);
}

#if 0
///
// TODO: Clean-up this entire main loop. It is excessively ugly.
int main(void)
{
    FSGame game;
    FSControl control;
    FSView gView = { .game = &game, .control = &control, .totalFramesDrawn = 0 };
    FSPSView pView = { .view = &gView };

    initSDL(&pView);
    fsGameInit(&game);
    fsParseIniFile(&pView, &gView, FS_CONFIG_FILENAME);

    // Loop until we didn't receive a restart
    do {
        pView.restart = false;

        // Wait till restart key is removed before entering loop.
        // REMOVE THIS PLEASE.
        while (1) {
            SDL_PumpEvents();
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            if (state[SDL_SCANCODE_RSHIFT] == 0) {
                break;
            }
            SDL_Delay(50);
        }

        // This is safe to call without overwriting user options.
        fsGameReset(&game);

        // Ideally we would store the parsed options somewhere and just reload
        // instead of re-reading the file.
        fsGameLoop(&pView, &gView);

        // If we finish a game successfully, check if the user wants to play again
        if (game.state == FSS_GAMEOVER) {
            // Allow FIELD macros to work
            FSPSView *v = &pView;

            // Well done
            renderString(v, "EXCELLENT", FIELD_X + FIELD_W / 2 - 40, FIELD_Y + FIELD_H / 2);

            // Blit and sleep in a loop so we don't get a non-redrawn screen
            for (int i = 0; i < (2000 / 50); ++i) {
                fsiBlit(v);
                SDL_Delay(50);
            }

            renderString(v, "(Y) TO PLAY AGAIN", FIELD_X + FIELD_W / 2 - 90, FIELD_Y + FIELD_H / 2 + 20);
            fsiBlit(v);
            while (1) {
                SDL_PumpEvents();
                const Uint8 *state = SDL_GetKeyboardState(NULL);
                if (state[SDL_SCANCODE_Y]) {
                    pView.restart = true;
                    break;
                }
                else if (state[SDL_GetScancodeFromKey(SDLK_q)] || SDL_QuitRequested()) {
                    // Just quit and don't restart
                    break;
                }

                // Stil want to blit here since when unminizing we need to redraw for example
                fsiBlit(v);
                SDL_Delay(50);
            }
        }

    } while (pView.restart == true);

    fsiFree(&pView);
}
#endif

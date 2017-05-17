#include <kernel/kbd.h>
#include <kernel/dt.h>
#include <kernel/com.h>
#include <kernel/timer.h>
#include <kernel/tty.h>

#include <faststack.h>

// Offset before the start of the field (hold space).
const int field_x_offset = 2 * 6;

// Main tetris engine.
FSEngine engine;

// Control options for the tetris engine.
FSControl control;

const int vk_keymap[FST_VK_COUNT] = {
    KEY_SPACE,          // FST_VK_UP
    KEY_ARROW_DOWN,     // FST_VK_DOWN
    KEY_ARROW_LEFT,     // FST_VK_LEFT
    KEY_ARROW_RIGHT,    // FST_VK_RIGHT
    KEY_Z,              // FST_VK_ROTL
    KEY_X,              // FST_VK_ROTR
    KEY_A,              // FST_VK_ROTH
    KEY_C,              // FST_VK_HOLD
    KEY_ENTER,          // FST_VK_START
    KEY_RSHIFT,         // FST_VK_RESTART
    0                   // FST_VK_QUIT
};

const int colormap[FS_NPT] = {
	VGA_COLOR_BLUE,
	VGA_COLOR_GREEN,
	VGA_COLOR_CYAN,
	VGA_COLOR_RED,
	VGA_COLOR_MAGENTA,
	VGA_COLOR_BROWN,
	VGA_COLOR_LIGHT_GREY
};

const int COLOR_DIM = 8;

uint32_t read_keys(void)
{
    uint32_t keys = 0;
    uint8_t state[128];
    kbd_state(state);

    for (int i = 0; i < FST_VK_COUNT; ++i) {
        if (state[vk_keymap[i]]) {
            keys |= FS_TO_FLAG(i);
        }
    }

    return keys;
}

static void draw_field(void)
{
    for (int y = engine.fieldHidden; y < engine.fieldHeight; ++y) {
        for (int i = 0; i < field_x_offset - 1; ++i) {
            ttyb_putc(' ');
        }
        ttyb_putc('|');

        for (int x = 0; x < engine.fieldWidth; ++x) {
            int tty_color;
            if (engine.b[y][x]) {
                tty_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
            }
            else {
                tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            }

            tty_set_color(tty_color);
            ttyb_puts("  ");
            tty_reset_color();
        }

        ttyb_puts("|\n");
    }

    // Bottom border
    for (int i = 0; i < field_x_offset - 1; ++i) {
        ttyb_putc(' ');
    }
    ttyb_putc('-');
    for (int x = 0; x < engine.fieldWidth; ++x) {
        ttyb_puts("--");
    }
    ttyb_puts("-\n");
}

static void draw_block(void)
{
    i8x2 blocks[4];
    int tty_color;

    if (engine.piece == FS_NONE) {
        return;
    }

    tty_color = vga_entry_color(VGA_COLOR_BLACK,
                                colormap[engine.piece]);
    tty_set_color(tty_color);

    // Draw block ghost
    fsGetBlocks(&engine, blocks, engine.piece, engine.x,
                engine.hardDropY - engine.fieldHidden, engine.theta);
    for (int i = 0; i < FS_NBP; ++i) {
        if (blocks[i].y < 0) {
            continue;
        }

        const int x_offset = 2 * blocks[i].x + field_x_offset;
        ttyb_putc_at(' ', x_offset, blocks[i].y);
        ttyb_putc_at(' ', x_offset + 1, blocks[i].y);
    }

    tty_color = vga_entry_color(VGA_COLOR_BLACK,
                                colormap[engine.piece] + COLOR_DIM);
    tty_set_color(tty_color);

    // Draw block
    fsGetBlocks(&engine, blocks, engine.piece, engine.x,
                engine.y - engine.fieldHidden, engine.theta);
    for (int i = 0; i < FS_NBP; ++i) {
        if (blocks[i].y < 0) {
            continue;
        }

        const int x_offset = 2 * blocks[i].x + field_x_offset;
        ttyb_putc_at(' ', x_offset, blocks[i].y);
        ttyb_putc_at(' ', x_offset + 1, blocks[i].y);
    }

    tty_reset_color();
}

static void draw_hold(void)
{
    i8x2 blocks[4];
    const int tty_color = vga_entry_color(VGA_COLOR_BLACK,
                                          colormap[engine.holdPiece]);
    tty_set_color(tty_color);

    if (engine.holdPiece != FS_NONE) {
        fsGetBlocks(&engine, blocks, engine.holdPiece, 0, 0, 0);
        for (int i = 0; i < FS_NBP; ++i) {
            const int x_offset = 2 + (engine.holdPiece == FS_I ||
                                        engine.holdPiece == FS_O ? 0 : 1);
            ttyb_putc_at(' ', x_offset + 2 * blocks[i].x, blocks[i].y);
            ttyb_putc_at(' ', x_offset + 2 * blocks[i].x + 1, blocks[i].y);
        }
    }

    tty_reset_color();
}

static void draw_preview(void)
{
    i8x2 blocks[4];

    const int preview_count = engine.nextPieceCount > FS_MAX_PREVIEW_COUNT ?
                                FS_MAX_PREVIEW_COUNT : engine.nextPieceCount;
    for (int i = 0; i < preview_count; ++i) {
        const int tty_color = vga_entry_color(VGA_COLOR_BLACK,
                                              colormap[engine.nextPiece[i]]);
        tty_set_color(tty_color);

        fsGetBlocks(&engine, blocks, engine.nextPiece[i], 0, 0, 0);
        const int x_offset = (engine.holdPiece == FS_I ||
                                engine.holdPiece == FS_O ? 0 : 1);
        for (int j = 0; j < FS_NBP; ++j) {
            const int x_ot = 2 * blocks[j].x + x_offset + field_x_offset + 2 * 11;
            const int y_ot = blocks[j].y + (4 * i);
            ttyb_putc_at(' ', x_ot, y_ot);
            ttyb_putc_at(' ', x_ot + 1, y_ot);
        }
    }

    tty_reset_color();
}

static void draw_target(void)
{
    const int remaining = engine.goal - engine.linesCleared > 0 ?
                            engine.goal - engine.linesCleared : 0;
    tty_set_cursor(field_x_offset + (2 * 10) / 2 - 1, engine.fieldHeight);
    ttyb_putc(remaining / 10 % 10 + '0');
    ttyb_putc(remaining % 10 + '0');
}

static void draw_status_text(void)
{
    const int x = field_x_offset + 9;
    const int y = 10;
    const int tty_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    tty_set_color(tty_color);
    switch (engine.state) {
        case FSS_READY:
            tty_set_cursor(x - 2, y);
            ttyb_puts("READY");
            break;
        case FSS_GO:
            tty_set_cursor(x, y);
            ttyb_puts("GO");
            break;
        case FSS_GAMEOVER:
            tty_set_cursor(x - 3, y);
            ttyb_puts("GAMEOVER");
            break;
    }

    tty_reset_color();
}

void draw(void)
{
    tty_clear_backbuffer();
    draw_field();
    draw_block();
    draw_hold();
    draw_preview();
    draw_target();
    draw_status_text();
    tty_flip();
}

void update(void)
{
    FSInput input = {0, 0, 0, 0, 0, 0};
    uint32_t keystate = read_keys();

    fsVirtualKeysToInput(&input, keystate, &engine, &control);
    fsGameTick(&engine, &input);
}

static void init_kernel(void)
{
    init_dt();
    init_timer();
    init_tty();
    init_kbd();
}

static void restart_game(void)
{
    engine.seed = timer_seed();
    fsGameInit(&engine);
}

void kernel_main(void)
{
    init_kernel();

    tty_clear();
    restart_game();

    while (1) {
        // Disallow quiting since we are the OS!
        if (engine.state == FSS_RESTART || engine.state == FSS_QUIT) {
            restart_game();
        }
        else if (engine.state == FSS_GAMEOVER) {
            hlt();  // Power-save until next input
        }

        update();
        draw();
        timer_sleep(16);
    }
}

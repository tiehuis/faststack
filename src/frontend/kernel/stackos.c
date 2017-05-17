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

// TODO: Make arrow keys work.
int vk_keymap[FST_VK_COUNT] = {
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
            if (engine.b[y][x]) {
                ttyb_puts("##");
            }
            else {
                ttyb_puts("  ");
            }
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

    const int tty_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    tty_set_color(tty_color);

    // Draw block ghost
    fsGetBlocks(&engine, blocks, engine.piece, engine.x,
                engine.hardDropY - engine.fieldHidden, engine.theta);
    for (int i = 0; i < FS_NBP; ++i) {
        if (blocks[i].y < 0) {
            continue;
        }

        const int x_offset = 2 * blocks[i].x + field_x_offset;
        ttyb_putc_at('@', x_offset, blocks[i].y);
        ttyb_putc_at('@', x_offset + 1, blocks[i].y);
    }

    // Draw block
    fsGetBlocks(&engine, blocks, engine.piece, engine.x,
                engine.y - engine.fieldHidden, engine.theta);
    for (int i = 0; i < FS_NBP; ++i) {
        if (blocks[i].y < 0) {
            continue;
        }

        const int x_offset = 2 * blocks[i].x + field_x_offset;
        ttyb_putc_at('@', x_offset, blocks[i].y);
        ttyb_putc_at('@', x_offset + 1, blocks[i].y);
    }

    tty_reset_color();
}

static void draw_hold(void)
{
    i8x2 blocks[4];

    const int tty_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    tty_set_color(tty_color);

    if (engine.holdPiece != FS_NONE) {
        fsGetBlocks(&engine, blocks, engine.holdPiece, 0, 0, 0);
        for (int i = 0; i < FS_NBP; ++i) {
            const int x_offset = 2 + (engine.holdPiece == FS_I ||
                                        engine.holdPiece == FS_O ? 0 : 1);
            ttyb_putc_at('#', x_offset + 2 * blocks[i].x, blocks[i].y);
            ttyb_putc_at('#', x_offset + 2 * blocks[i].x + 1, blocks[i].y);
        }
    }

    tty_reset_color();
}

static void draw_preview(void)
{
    i8x2 blocks[4];

    const int tty_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    tty_set_color(tty_color);

    const int preview_count = engine.nextPieceCount > FS_MAX_PREVIEW_COUNT ?
                                FS_MAX_PREVIEW_COUNT : engine.nextPieceCount;
    for (int i = 0; i < preview_count; ++i) {
        fsGetBlocks(&engine, blocks, engine.nextPiece[i], 0, 0, 0);
        const int x_offset = (engine.holdPiece == FS_I ||
                                engine.holdPiece == FS_O ? 0 : 1);
        for (int j = 0; j < FS_NBP; ++j) {
            const int x_ot = 2 * blocks[j].x + x_offset + field_x_offset + 2 * 11;
            const int y_ot = blocks[j].y + (4 * i);
            ttyb_putc_at('#', x_ot, y_ot);
            ttyb_putc_at('#', x_ot + 1, y_ot);
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

void draw(void)
{
    tty_clear_backbuffer();
    draw_field();
    draw_block();
    draw_hold();
    draw_preview();
    draw_target();
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

void kernel_main(void)
{
    init_kernel();

    tty_clear();
    fsGameInit(&engine);

    // TODO: Restart does not refresh screen as we would like.
    while (1) {
        // Disallow quiting since we are the OS!
        if (engine.state == FSS_RESTART || engine.state == FSS_QUIT) {
            fsGameInit(&engine);
        }
        else if (engine.state == FSS_GAMEOVER) {
            hlt();  // Power-save until next input
        }

        update();
        draw();
        timer_sleep(16);
    }
}

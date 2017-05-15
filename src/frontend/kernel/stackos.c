#include <kernel/kbd.h>
#include <kernel/dt.h>
#include <kernel/com.h>
#include <kernel/timer.h>
#include <kernel/tty.h>

#include <faststack.h>

// Main tetris engine.
FSEngine engine;

// Control options for the tetris engine.
FSControl control;

int vk_keymap[FST_VK_COUNT] = {
    KEY_SPACE,          // FST_VK_UP
    KEY_ARROW_DOWN,     // FST_VK_DOWN
    KEY_ARROW_LEFT,     // FST_VK_LEFT
    KEY_ARROW_RIGHT,    // FST_VK_RIGHT
    KEY_Z,              // FST_VK_ROTL
    KEY_X,              // FST_VK_ROTR
    KEY_A,              // FST_VK_ROTH
    KEY_C,              // FST_VK_HOLD
    KEY_P, // Make enter!          // FST_VK_START
    KEY_RSHIFT,         // FST_VK_RESTART
    KEY_Q               // FST_VK_QUIT
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

// TODO: Buffer draws and don't always redraw.
void draw(void)
{
    tty_clear();

    // Draw field.
    for (int y = engine.fieldHidden; y < engine.fieldHeight; ++y) {
        for (int x = 0; x < engine.fieldWidth; ++x) {
            if (engine.b[y][x]) {
                tty_puts("##");
            }
            else {
                tty_puts("  ");
            }
        }

        tty_putc('\n');
    }

    i8x2 blocks[4];
    fsGetBlocks(&engine, blocks, engine.piece, engine.x,
                engine.y - engine.fieldHidden, engine.theta);
    for (int i = 0; i < FS_NBP; ++i) {
        if (blocks[i].y < 0) {
            continue;
        }

        const int tty_color = vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK);
        tty_putc_at('@', tty_color, 2 * blocks[i].x    , blocks[i].y);
        tty_putc_at('@', tty_color, 2 * blocks[i].x + 1, blocks[i].y);
    }
}

void update(void)
{
    FSInput input = {0, 0, 0, 0, 0, 0};
    uint32_t keystate = read_keys();

    fsVirtualKeysToInput(&input, keystate, &engine, &control);
    fsGameTick(&engine, &input);

    static int i = 0;
    com_puts("update!: ");
    com_put_hex(i++);
    com_puts("\n");
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
    fsGameInit(&engine);

    while (1) {
        update();
        draw();
        timer_sleep(16);
    }
}

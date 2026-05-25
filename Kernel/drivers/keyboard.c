#include "keyboard.h"
#include "defs.h"

#define KB_BUFFER_SIZE 256

static char kb_buffer[KB_BUFFER_SIZE];
static int kb_read_idx = 0;
static int kb_write_idx = 0;
static int kb_count = 0;

static int shift_pressed = 0;
static int caps_lock = 0;
static int ctrl_pressed = 0;

static const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_shift[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

void keyboard_init(void) {
    kb_read_idx = 0;
    kb_write_idx = 0;
    kb_count = 0;
    shift_pressed = 0;
    caps_lock = 0;
    ctrl_pressed = 0;
}

static void buffer_put(char c) {
    if (kb_count < KB_BUFFER_SIZE) {
        kb_buffer[kb_write_idx] = c;
        kb_write_idx = (kb_write_idx + 1) % KB_BUFFER_SIZE;
        kb_count++;
    }
}

void keyboard_handler(uint8_t scancode) {
    /* Key release */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36)
            shift_pressed = 0;
        if (released == 0x1D)
            ctrl_pressed = 0;
        return;
    }

    /* Special keys */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0x1D) {
        ctrl_pressed = 1;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    /* Ctrl+C → store as 0x03 */
    if (ctrl_pressed && scancode == 0x2E) {
        buffer_put(0x03);
        return;
    }
    /* Ctrl+D → store as 0x04 */
    if (ctrl_pressed && scancode == 0x20) {
        buffer_put(0x04);
        return;
    }

    if (scancode >= sizeof(scancode_to_ascii))
        return;

    char c;
    if (shift_pressed)
        c = scancode_to_ascii_shift[scancode];
    else
        c = scancode_to_ascii[scancode];

    if (c == 0)
        return;

    /* Apply caps lock to letters */
    if (caps_lock && c >= 'a' && c <= 'z')
        c -= 32;
    else if (caps_lock && c >= 'A' && c <= 'Z')
        c += 32;

    buffer_put(c);
}

char keyboard_get_char(void) {
    while (kb_count == 0) {
        __asm__("sti");
        __asm__("hlt");
    }

    char c = kb_buffer[kb_read_idx];
    kb_read_idx = (kb_read_idx + 1) % KB_BUFFER_SIZE;
    kb_count--;
    return c;
}

int keyboard_has_char(void) {
    return kb_count > 0;
}

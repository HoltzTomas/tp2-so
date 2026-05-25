#include "keyboard.h"
#include "process.h"
#include "naiveConsole.h"
#include "interrupts.h"

#define KB_BUFFER_SIZE 256

static char buffer[KB_BUFFER_SIZE];
static int readIdx = 0;
static int writeIdx = 0;
static int count = 0;
static pid_t blockedPid = -1;
static int eofFlag = 0;
static int ctrlPressed = 0;

static const char scancodeToAscii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '};

void keyboard_init(void) {
    readIdx = 0;
    writeIdx = 0;
    count = 0;
    blockedPid = -1;
    eofFlag = 0;
    ctrlPressed = 0;
}

void keyboard_add_char(uint8_t scancode) {
    if (scancode == 0x1D) {
        ctrlPressed = 1;
        return;
    }
    if (scancode == 0x9D) {
        ctrlPressed = 0;
        return;
    }

    if (scancode & 0x80)
        return;

    if (ctrlPressed) {
        if (scancode == 0x20) {
            keyboard_send_eof();
            return;
        }
        if (scancode == 0x2E) {
            keyboard_ctrl_c();
            return;
        }
        return;
    }

    if (scancode >= sizeof(scancodeToAscii))
        return;

    char c = scancodeToAscii[scancode];
    if (c == 0)
        return;

    if (count < KB_BUFFER_SIZE) {
        buffer[writeIdx] = c;
        writeIdx = (writeIdx + 1) % KB_BUFFER_SIZE;
        count++;

        if (blockedPid != -1) {
            process_unblock(blockedPid);
            blockedPid = -1;
        }
    }
}

char keyboard_get_char(void) {
    while (count == 0 && !eofFlag) {
        blockedPid = process_getpid();
        process_block(process_getpid());
    }

    if (eofFlag && count == 0) {
        eofFlag = 0;
        return (char)EOF;
    }

    char c = buffer[readIdx];
    readIdx = (readIdx + 1) % KB_BUFFER_SIZE;
    count--;
    return c;
}

int keyboard_has_char(void) {
    return count > 0 || eofFlag;
}

void keyboard_send_eof(void) {
    eofFlag = 1;
    if (blockedPid != -1) {
        process_unblock(blockedPid);
        blockedPid = -1;
    }
}

void keyboard_ctrl_c(void) {
    pid_t fg = process_get_foreground();
    if (fg > 1) {
        process_kill(fg);
    }
}

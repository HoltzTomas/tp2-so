#include <stdint.h>

extern char bss;
extern char endOfBinary;

int main(void);

void *memset(void *dest, int32_t c, uint64_t length);

int _start(void) {
    memset(&bss, 0, &endOfBinary - &bss);
    return main();
}

#include <stdint.h>
#include "lib.h"
#include "moduleLoader.h"
#include "naiveConsole.h"
#include "interrupts.h"
#include "keyboard.h"
#include "video.h"
#include "memoryManager.h"
#include "defs.h"

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void *const sampleCodeModuleAddress = (void *)0x400000;
static void *const sampleDataModuleAddress = (void *)0x500000;

static void *const heapBase = (void *)0x600000;
#define HEAP_SIZE (64 * 1024 * 1024)

typedef int (*EntryPoint)(void);

void clearBSS(void *bssAddress, uint64_t bssSize) {
    memset(bssAddress, 0, bssSize);
}

void *getStackBase(void) {
    return (void *)(
        (uint64_t)&endOfKernel
        + PageSize * 8
        - sizeof(uint64_t));
}

void *initializeKernelBinary(void) {
    char buffer[10];

    ncPrint("[x64BareBones]");
    ncNewline();

    ncPrint("CPU Vendor:");
    ncPrint(cpuVendor(buffer));
    ncNewline();

    ncPrint("[Loading modules]");
    ncNewline();
    void *moduleAddresses[] = {
        sampleCodeModuleAddress,
        sampleDataModuleAddress};

    loadModules(&endOfKernelBinary, moduleAddresses);
    ncPrint("[Done]");
    ncNewline();
    ncNewline();

    ncPrint("[Initializing kernel's binary]");
    ncNewline();

    clearBSS(&bss, &endOfKernel - &bss);

    ncPrint("  text: 0x");
    ncPrintHex((uint64_t)&text);
    ncNewline();
    ncPrint("  rodata: 0x");
    ncPrintHex((uint64_t)&rodata);
    ncNewline();
    ncPrint("  data: 0x");
    ncPrintHex((uint64_t)&data);
    ncNewline();
    ncPrint("  bss: 0x");
    ncPrintHex((uint64_t)&bss);
    ncNewline();

    ncPrint("[Done]");
    ncNewline();
    ncNewline();
    return getStackBase();
}

int main(void) {
    ncPrint("[Kernel Main]");
    ncNewline();

    ncPrint("[Initializing memory manager]");
    ncNewline();
    mm_init(heapBase, HEAP_SIZE);

    ncPrint("[Initializing keyboard]");
    ncNewline();
    keyboard_init();

    ncPrint("[Initializing video]");
    ncNewline();
    video_init();

    ncPrint("[Initializing interrupts]");
    ncNewline();
    irq_init();
    irq_enable();

    ncPrint("[Jumping to userland]");
    ncNewline();

    ((EntryPoint)sampleCodeModuleAddress)();

    return 0;
}

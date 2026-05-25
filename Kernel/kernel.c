#include <stdint.h>
#include "lib.h"
#include "moduleLoader.h"
#include "naiveConsole.h"
#include "interrupts.h"
#include "process.h"
#include "scheduler.h"
#include "memoryManager.h"
#include "semaphore.h"
#include "pipe.h"
#include "keyboard.h"
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
    ncPrint("[x64BareBones]");
    ncNewline();

    char buffer[10];
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

static uint64_t shell_main(uint64_t argc, char *argv[]) {
    ((int (*)(void))sampleCodeModuleAddress)();
    return 0;
}

int main(void) {
    ncPrint("[Kernel Main]");
    ncNewline();

    ncPrint("[Initializing memory manager]");
    ncNewline();
    mm_init(heapBase, HEAP_SIZE);

    ncPrint("[Initializing process module]");
    ncNewline();
    process_init();
    scheduler_init();

    ncPrint("[Initializing semaphores]");
    ncNewline();
    sem_init_module();

    ncPrint("[Initializing pipes]");
    ncNewline();
    pipe_init_module();

    ncPrint("[Initializing keyboard]");
    ncNewline();
    keyboard_init();

    ncPrint("[Initializing interrupts]");
    ncNewline();
    irq_init();

    ncPrint("[Creating shell process]");
    ncNewline();
    fd_t fds[2] = {STDIN, STDOUT};
    process_create(shell_main, 0, (char *[]){NULL}, "shell", 1, fds);

    ncPrint("[Starting scheduler]");
    ncNewline();
    irq_enable();
    force_timer();

    while (1)
        _hlt();

    return 0;
}

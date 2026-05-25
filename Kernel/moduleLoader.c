#include <stdint.h>
#include "moduleLoader.h"
#include "lib.h"

static void loadModule(uint8_t **module, void *targetModuleAddress);
static uint32_t readUint32(uint8_t **address);

void loadModules(void *payloadStart, void **moduleTargetAddresses) {
    int i = 0;
    uint8_t *currentModule = (uint8_t *)payloadStart;
    uint32_t moduleCount = readUint32(&currentModule);

    while (i < (int)moduleCount) {
        loadModule(&currentModule, moduleTargetAddresses[i]);
        i++;
    }
}

static void loadModule(uint8_t **module, void *targetModuleAddress) {
    uint32_t moduleSize = readUint32(module);
    memcpy(targetModuleAddress, *module, moduleSize);
    *module += moduleSize;
}

static uint32_t readUint32(uint8_t **address) {
    uint32_t result = *(uint32_t *)(*address);
    *address += sizeof(uint32_t);
    return result;
}

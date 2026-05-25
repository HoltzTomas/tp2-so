#ifndef MODULE_LOADER_H
#define MODULE_LOADER_H

#include <stdint.h>

void loadModules(void *payloadStart, void **moduleTargetAddresses);

#endif

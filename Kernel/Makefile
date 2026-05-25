include Makefile.inc

MM?=FREE_LIST

KERNEL=kernel.bin
SOURCES=$(wildcard *.c) $(wildcard drivers/*.c) $(wildcard process/*.c) $(wildcard ipc/*.c) $(wildcard syscalls/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm
LOADEROBJECT=$(LOADERSRC:.asm=.o)

ifeq ($(MM),BUDDY)
SOURCES_MM=memory/buddyMemoryManager.c
else
SOURCES_MM=memory/memoryManager.c
endif

OBJECTS_MM=$(SOURCES_MM:.c=.o)

STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_MM) $(OBJECTS_ASM) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_MM) $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -D$(MM) -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o *.o *.bin drivers/*.o memory/*.o process/*.o ipc/*.o syscalls/*.o

.PHONY: all clean

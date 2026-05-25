include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c) $(wildcard commands/*.c) $(wildcard tests/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADER_OBJ=_loader.o

all: $(LOADER_OBJ) $(OBJECTS) $(OBJECTS_ASM)
	$(LD) -T sampleCodeModule.ld -o ../$(MODULE) $(LOADER_OBJ) $(OBJECTS) $(OBJECTS_ASM)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADER_OBJ): _loader.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

clean:
	rm -rf *.o commands/*.o tests/*.o asm/*.o

.PHONY: all clean

include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c) $(wildcard tests/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)

all: $(MODULE)

$(MODULE): $(OBJECTS) $(OBJECTS_ASM)
	$(GCC) $(GCCFLAGS) -T sampleCodeModule.ld -I./include _loader.c $(OBJECTS) $(OBJECTS_ASM) -o ../$(MODULE)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf *.o tests/*.o asm/*.o

.PHONY: all clean

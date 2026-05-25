MM=FREE_LIST

all: toolchain bootloader kernel userland image

toolchain:
	cd Toolchain; make all

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all MM=$(MM)

userland:
	cd Userland; make all

buddy:
	$(MAKE) all MM=BUDDY

image: kernel bootloader userland
	cd Image; make all

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Userland; make clean

.PHONY: toolchain bootloader image kernel userland all clean buddy

MM=FREE_LIST

all: bootloader kernel userland image

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all MM=$(MM)

userland:
	cd Userland; make all

image: kernel bootloader userland
	cd Image; make all

buddy:
	make all MM=BUDDY

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Userland; make clean

.PHONY: bootloader image kernel userland all clean buddy

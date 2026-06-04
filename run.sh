#!/bin/bash
# Build and run the OS in QEMU
docker run --rm -v "$(pwd)":/root -w /root agodio/itba-so-multiarch:3.1 make all
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512

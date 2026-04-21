#!/bin/sh


make clean
make all "$@"
sh ./syms_file_maker.sh kernel.elf syms.bin
make run "$@"
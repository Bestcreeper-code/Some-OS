#!/bin/sh

set -e

make all "$@"
sh ./syms_file_maker.sh kernel.elf syms.bin
make run "$@"

printf "\e[0m"
#!/bin/bash
# Converts ELF symbols into a compact binary table for kernel usage
# Format: [uint32 addr][char type][uint16 name_len][name][\0]

set -euo pipefail

ELF_FILE="${1:-}"
OUT_FILE="${2:-}"

if [[ -z "$ELF_FILE" || ! -f "$ELF_FILE" ]]; then
    echo "Usage: $0 <file.elf> [output.syms]"
    exit 1
fi

if [[ -z "$OUT_FILE" ]]; then
    OUT_FILE="${ELF_FILE%.*}.syms"
fi

echo "Generating binary symbol table from '$ELF_FILE' -> '$OUT_FILE'"

# Clear/create output
: > "$OUT_FILE"

# Process symbols from nm
nm -n --defined-only "$ELF_FILE" | awk '{print $1, $2, $3}' | while read -r addr type name; do
    [[ -z "$addr" || -z "$name" ]] && continue

    # Convert hex addr → 4-byte little endian
    addr_bin=$(printf "%08x" $((16#$addr)) | sed 's/\(..\)/\1 /g')
    for b in $addr_bin; do
        printf "\\x$b" >> "$OUT_FILE"
    done

    # Write 1-byte type
    printf "%c" "$type" >> "$OUT_FILE"

    # Write 2-byte little-endian name length
    name_len=${#name}
    printf "\\x$(printf '%02x' $((name_len & 0xff)))\\x$(printf '%02x' $((name_len >> 8)))" >> "$OUT_FILE"

    # Write name + null terminator
    printf "%s\0" "$name" >> "$OUT_FILE"
done

# Append null symbol to terminate array
printf "\x00\x00\x00\x00\x00\x00\x00" >> "$OUT_FILE"

echo "Wrote: $OUT_FILE"

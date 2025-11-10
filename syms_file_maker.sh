#!/usr/bin/env bash
set -euo pipefail
[[ $# -lt 2 ]] && { echo "Usage: $0 <input.elf> <output.bin> [--demangle]"; exit 1; }

ELF="$1"; OUT="$2"; DEMANGLE="${3:-}"
command -v nm >/dev/null || { echo "nm not found"; exit 1; }
[[ "$DEMANGLE" == "--demangle" ]] && command -v c++filt >/dev/null || true

> "$OUT"
nm -n --defined-only "$ELF" | while read -r addr type name; do
    [[ -z "$addr" || -z "$type" || -z "$name" ]] && continue
    [[ "$type" == "U" ]] && continue
    [[ ! "$addr" =~ ^[0-9A-Fa-f]+$ ]] && continue
    [[ "$DEMANGLE" == "--demangle" ]] && name="$(echo "$name" | c++filt)"
    perl -e '
        use strict; use warnings;
        my ($addr,$type,$name)=@ARGV;
        my $len=length($name)+1;
        print pack("V C v", hex($addr), ord($type), $len), $name, "\0";
    ' "$addr" "$type" "$name" >>"$OUT"
done

echo "Wrote $(stat -c%s "$OUT") bytes to $OUT"

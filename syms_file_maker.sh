#!/usr/bin/env bash
set -euo pipefail

[[ $# -lt 3 ]] && {
  echo "Usage: $0 <input.elf> <output.bin> <x86|x86_64> [--demangle]"
  exit 1
}

ELF="$1"
OUT="$2"
ARCH="$3"
DEMANGLE="${4:-}"

command -v nm >/dev/null || { echo "nm not found"; exit 1; }
[[ "$DEMANGLE" == "--demangle" ]] && command -v c++filt >/dev/null || true

> "$OUT"

case "$ARCH" in
  x86)     PACK="V"   ;;   # 32-bit
  x86_64)  PACK="Q<"  ;;   # 64-bit little-endian
  *) echo "Invalid arch: use x86 or x86_64"; exit 1 ;;
esac

nm -n --defined-only "$ELF" | while read -r addr type name; do
    [[ -z "$addr" || -z "$type" || -z "$name" ]] && continue
    [[ "$type" == "U" ]] && continue
    [[ ! "$addr" =~ ^[0-9A-Fa-f]+$ ]] && continue

    if [[ "$DEMANGLE" == "--demangle" ]]; then
        name="$(echo "$name" | c++filt)"
    fi

    perl -e '
        use strict; use warnings;
        my ($addr,$type,$name,$pack)=@ARGV;
        my $len = length($name) + 1;
        print pack($pack . " C v", hex($addr), ord($type), $len), $name, "\0";
    ' "$addr" "$type" "$name" "$PACK" >>"$OUT"
done

echo "Wrote $(stat -c%s "$OUT") bytes to $OUT"
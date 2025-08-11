#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: $0 input.txt output.c"
  exit 1
fi

input="$1"
output="$2"

# Read all hex bytes into an array (ignore non-hex characters)
bytes=($(grep -oE '[0-9A-Fa-f]{2}' "$input"))

echo "unsigned char data[] = {" > "$output"

count=0
line=""

for byte in "${bytes[@]}"; do
  line+="0x${byte}, "
  ((count++))
  if (( count % 16 == 0 )); then
    echo "    $line" >> "$output"
    line=""
  fi
done

# Print any remaining bytes (last line)
if [ -n "$line" ]; then
  echo "    $line" >> "$output"
fi

echo "};" >> "$output"

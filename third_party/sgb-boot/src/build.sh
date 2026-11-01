#!/bin/sh
# Rebuild the clean-room SGB boot ROM.  Optional: the normal CMake build uses
# ../bootroms/sgb_boot.bin and never invokes RGBDS.
set -eu

cd "$(dirname "$0")"
command -v rgbasm >/dev/null 2>&1 || {
    echo "error: rgbasm is required only for rebuilding the boot ROM" >&2
    exit 1
}
command -v rgblink >/dev/null 2>&1 || {
    echo "error: rgblink is required only for rebuilding the boot ROM" >&2
    exit 1
}

rgbasm -o sgb_boot.o sgb_boot.asm
rgblink -x -o ../bootroms/sgb_boot.bin sgb_boot.o
rm -f sgb_boot.o

actual=$(wc -c < ../bootroms/sgb_boot.bin | tr -d ' ')
if [ "$actual" -ne 256 ]; then
    echo "error: sgb_boot.bin is $actual bytes, expected 256" >&2
    exit 1
fi

echo "rebuilt:"
shasum -a 256 ../bootroms/sgb_boot.bin

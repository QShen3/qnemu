#!/bin/sh
# Rebuild the prebuilt Bootix DMG boot ROM.  Optional: the normal CMake build
# uses ../bootroms/dmg_boot.bin and never invokes RGBDS.
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

# Bootix's original 1997-era hardware.inc is not accepted by RGBDS 1.x.
# Use the modern hardware.inc that already ships with SameBoy (MIT).
rgbasm -I ../../sameboy/src -o dmg_boot.o bootix_dmg.asm
rgblink -t -w -x -o ../bootroms/dmg_boot.bin dmg_boot.o
rm -f dmg_boot.o

actual=$(wc -c < ../bootroms/dmg_boot.bin | tr -d ' ')
if [ "$actual" -ne 256 ]; then
    echo "error: dmg_boot.bin is $actual bytes, expected 256" >&2
    exit 1
fi

echo "rebuilt:"
shasum -a 256 ../bootroms/dmg_boot.bin

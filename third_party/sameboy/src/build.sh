#!/bin/sh
# Rebuild the prebuilt CGB boot ROM from the source in this directory.
#
# This script is optional: qnemu's normal CMake build uses the committed
# binary in ../bootroms/ and never invokes RGBDS. Run this only when you
# intentionally change the boot ROM source or the QNEMU wordmark.
set -eu

cd "$(dirname "$0")"

for tool in rgbasm rgblink cc python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "error: $tool is required only for rebuilding the boot ROMs" >&2
        exit 1
    fi
done

cc -std=c99 -Wall -Werror pb12.c -o pb12
python3 gen_qnemu_logo.py
./pb12 < qnemu_logo.2bpp > qnemu_logo.pb12
rm -f pb12

for model in cgb; do
    rgbasm -o "${model}_boot.o" "${model}_boot.asm"
    rgblink -x -o "../bootroms/${model}_boot.bin" "${model}_boot.o"
    rm -f "${model}_boot.o"
done

check_size() {
    actual=$(wc -c < "$1" | tr -d ' ')
    if [ "$actual" -ne "$2" ]; then
        echo "error: $1 is $actual bytes, expected $2" >&2
        exit 1
    fi
}

check_size ../bootroms/cgb_boot.bin 2304

echo "rebuilt:"
shasum -a 256 ../bootroms/*.bin

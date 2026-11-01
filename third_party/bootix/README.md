# Bootix DMG boot ROM

qnemu ships Bootix's copyright-free DMG boot ROM as
`bootroms/dmg_boot.bin`.

- Upstream: <https://github.com/Ashiepaws/Bootix>
- Commit: `e2121d0f2e5665efd87b53b53d16ea61775898f3`
- License: CC0-1.0 (public domain dedication), see [LICENSE](LICENSE)
- File: 256 bytes, SHA-256 in `bootroms/SHA256SUMS`

Bootix displays the logo from the **cartridge header** (`$0104-$0133`) with
its own scroll/fade animation and chime, exactly like a DMG boot ROM. It does
not contain a QNEMU logo and does not verify the header checksum or logo.
The `BOOTIX.DMG v1.2` string at the end is part of the upstream ROM.

## Source notes

`src/bootix_dmg.asm` is the upstream DMG source with two mechanical changes
for RGBDS 1.x:

- the four `EQU` definitions use `DEF ... EQU`;
- the include resolves the modern `hardware.inc` from
  `third_party/sameboy/src` instead of the repository's 1997-era copy.

The original Bootix `hardware.inc` is not compatible with RGBDS 1.x. The
modern header is MIT-licensed; its notice is in
`third_party/sameboy/LICENSE`.

Rebuilding is optional and needs RGBDS:

```sh
cd third_party/bootix/src
./build.sh
```

The normal CMake build uses the committed binary and does not require RGBDS.

## Scope

This is a DMG/MGB boot ROM. qnemu uses it only for the **DMG** model; CGB
mode uses the QNEMU CGB boot ROM, and SGB continues to use the HLE
`--boot-model` path. Bootix's displayed logo is the game's cartridge-header
logo, not a qnemu wordmark.

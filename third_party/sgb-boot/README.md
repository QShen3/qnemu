# qnemu SGB boot ROM

`bootroms/sgb_boot.bin` is qnemu's clean-room, 256-byte Super Game Boy
GB-side boot ROM. It is written from the documented SGB packet protocol
(Pan Docs), not copied from Nintendo's ROM or SameBoy's SGB image.

- License: MIT (see [LICENSE](LICENSE))
- Size: 256 bytes; SHA-256 in `bootroms/SHA256SUMS`
- Source: `src/sgb_boot.asm`
- Rebuild: `cd src && ./build.sh` (optional; needs RGBDS)

## Behaviour

The SGB's GB-side boot ROM is a header forwarder, not a graphical boot
screen:

1. Initialize SP, LCD and the documented SGB post-boot APU registers.
2. Send six packets with magic bytes `F1,F3,F5,F7,F9,FB` to the SNES-side
   SGB BIOS through `FF00`. Each packet carries 14 header bytes from
   `$0104` plus a checksum, followed by four VBlanks.
3. Set the post-boot SGB registers (`A=$01`, `F=0`, `B=0`, `C=$14`, `D=0`,
   `E=0`, `H=$C0`, `L=$60`) and unmap at `FF50`.

It does not display a logo or verify the header checksum. The visible SGB
border, palette menus and boot animation belong to the **SNES-side SGB
BIOS**; qnemu keeps those in `GbSgb`'s HLE. `GbSgb::commandReady()` ignores
the `F1..FB` header packets because `Gb::loadCartridge()` has already called
`setHeader()`.

## Tests

`boot_regs-sgb` and `boot_hwio-S` pass with this ROM. `boot_div-S` is not a
pass criterion because it asserts the official SGB ROM's exact cycle count;
this implementation sends the same packets but does not reproduce the
official timing.

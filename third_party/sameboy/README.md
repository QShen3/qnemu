# SameBoy CGB boot ROM

qnemu ships one prebuilt boot ROM, `bootroms/cgb_boot.bin`, derived from
[SameBoy](https://github.com/LIJI32/SameBoy)'s free CGB boot ROM
reimplementation.

- Upstream commit: `19a7b14cfb10bb20b0f931b3d77359c11f866e17`
  (2026-10-28, SameBoy 1.0.4)
- Upstream path: `BootROMs/cgb_boot.asm`
- License: Expat/MIT, see [LICENSE](LICENSE)
- Copyright (c) 2015-2026 Lior Halphon

The upstream `LICENSE` is included verbatim. Its iOS/HexFiend exception does
not apply to the boot ROM used here. The unused `NINTENDO_LOGO` macro was
removed from `src/hardware.inc` so the repository does not carry the logo
bitmap.

## Why only the CGB boot ROM?

While preparing this directory, qnemu's boot ROM guard found that SameBoy's
DMG and SGB boot ROMs assemble to binaries that are **byte-identical to the
official Nintendo boot ROMs**:

| SameBoy source | SHA-1 | Matches |
| --- | --- | --- |
| `dmg_boot.asm` | `1db57a1e8b6e4096f811587f9eab0c6675fd9755` | Official DMG |
| `sgb_boot.asm` | `369e6eb5e0c975eaa52a4a3f6ee07b2a3c3c16de` | Official SGB |

Even though SameBoy publishes those files under the Expat license, a
byte-identical copy is still Nintendo's copyrighted ROM. qnemu therefore does
**not** redistribute them. DMG and SGB modes continue to use the existing
HLE `--boot-model` path. If you want a DMG boot animation, use a dump from
your own hardware locally and do not commit it.

The CGB image is different: it is SameBoy's CGB boot ROM reimplementation,
not a byte copy. Its pre-modification SHA-1 is not an official CGB hash, and
qnemu's copy has been further modified as described below.

## File

| File | Size | Notes |
| --- | ---: | --- |
| `bootroms/cgb_boot.bin` | 2304 B | QNEMU-logo CGB boot ROM |

`cgb_boot.bin` uses the standard CGB dump layout: `$0000-$00FF`, the
`$0100-$01FF` cartridge-header gap, then `$0200-$08FF`.

## CGB modification

The CGB boot ROM's custom logo was SameBoy's own logo. It has been replaced
with a 128x24 QNEMU wordmark (16x3 tiles), and the tilemap fill was simplified
to write tile IDs 8-55 sequentially. The original per-tile intro animation was
removed; the boot chime and post-chime wait are unchanged. The compatibility
palette logic, title checksum handling, DMG-mode setup, and hand-off values are
unchanged.

The ROM still reads the Nintendo logo from the cartridge header for DMG
compatibility, as the hardware boot ROM does; it does not embed the full
Nintendo logo. That is a compatibility behaviour, not qnemu branding.

## Rebuilding (optional)

The committed binary is what qnemu uses. **RGBDS is not a qnemu build
dependency**; the normal CMake build never invokes it. To rebuild after
changing the logo or the source:

```sh
cd third_party/sameboy/src
./build.sh
```

This needs RGBDS (`rgbasm`/`rgblink`) and a C compiler for the small PB12
compressor. The script regenerates `qnemu_logo.2bpp` and `qnemu_logo.pb12`,
assembles the ROM, checks its size, and updates `bootroms/`. Update
`bootroms/SHA256SUMS` afterwards.

## Validation

The CGB ROM was checked with SameBoy's tester against Mooneye's
`misc/boot_regs-cgb.gb` (all register assertions pass), and the QNEMU wordmark
was verified visually.

## Official boot ROMs

No Nintendo boot ROMs are stored in this repository. Do not commit dumps,
including renamed ones. `tests/rom/check_official_boot_roms.py` rejects tracked
files whose hashes match known official DMG/MGB/SGB/SGB2/CGB/CGB0/AGB dumps; CI
runs it on every push and pull request.

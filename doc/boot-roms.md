# Boot ROMs

qnemu ships three prebuilt, free boot ROMs:

| File | Model | Notes |
| --- | --- | --- |
| `dmg_boot.bin` | DMG | Bootix (CC0), 256 B; displays the cartridge-header logo |
| `cgb_boot.bin` | CGB | SameBoy CGB reimplementation with the QNEMU wordmark |
| `sgb_boot.bin` | SGB | qnemu clean-room SGB header forwarder (MIT), 256 B |

The CGB image is derived from [SameBoy](https://github.com/LIJI32/SameBoy)'s
Expat/MIT-licensed CGB boot ROM. The DMG image is derived from
[Bootix](https://github.com/Ashiepaws/Bootix), which is CC0. The SGB image is
qnemu's own implementation of the documented SGB packet protocol. See
`third_party/sameboy/README.md`, `third_party/bootix/README.md`,
`third_party/sgb-boot/README.md` and their `LICENSE` files.

SameBoy's DMG and SGB boot ROMs are **not** shipped: they assemble to binaries
that are byte-identical to Nintendo's official DMG/SGB boot ROMs. Do not commit
dumps of your own; use them locally only.

All three binaries are committed, so the normal build needs **no RGBDS
dependency**. Rebuilding is optional and documented in the corresponding
`src/build.sh` scripts.

## Startup behaviour

Every model runs its boot ROM at startup; there is no separate HLE post-boot
path in the emulator:

- DMG runs Bootix and shows the cartridge-header logo (not a QNEMU wordmark).
- CGB runs the QNEMU image. CGB-capable cartridges stay in CGB mode, while
  DMG-only cartridges switch to DMG compatibility through KEY0 and keep the CGB
  PPU registers available. The ROM picks the per-game compatibility palette;
  hold a direction, optionally with A/B, while the logo is shown to choose one
  of the 16 manual combinations.
- SGB runs the clean-room header forwarder, then hands off to the HLE SGB
  BIOS. The border/palette menus remain HLE; this ROM does not add a fake
  SNES-side boot animation.

The only post-boot shortcut left is the test runner's `--boot-model`, which
reproduces official model-specific CPU/DIV/I-O state for Mooneye tests.

## Test status

The boot ROMs pass Mooneye `boot_regs-cgb`, `boot_hwio-C`,
`boot_regs-dmgABC`, `boot_regs-sgb`, `boot_hwio-S`, and the CGB/DMG acid2
image tests. `boot_div-*` and `boot_hwio-dmgABCmgb` are not registered: they
assert the official ROMs' exact cycle counts/DIV values, which these
reimplementations do not reproduce.

CI runs `tests/rom/check_official_boot_roms.py`, which rejects tracked files
matching known official DMG/MGB/SGB/SGB2/CGB/CGB0/AGB boot ROMs.

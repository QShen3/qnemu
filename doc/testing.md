# Testing QNEmu

QNEmu has a 300-case unit suite plus 168 curated test ROMs (Blargg, Mooneye
and acid2) registered with CTest. The curated set registers only passing ROMs;
every registered ROM test passes.

## Unit tests

```bash
ctest --preset debug
./build/tests/qnemu_test --gtest_filter='GbChannel*:GbApu*'
```

The unit suite covers the CPU, PPU/FIFO, APU, timer, joypad, interrupt
controller, memory, MMU/CPU bus, cartridge/MBC code, serial link, infrared
and the save manager.

## Test ROM runner

`gb_test_runner` runs a ROM headlessly with a small test bus (ROM/MBC, RAM,
timer, IF/IE, serial and a line counter for VBlank). It understands both common
result protocols:

- Blargg serial output and the Blargg screen console (`Passed` / `Failed`).
- The Mooneye `LD B, B` breakpoint with the Fibonacci register signature.

```bash
./build/tests/rom/gb_test_runner path/to/test.gb
./build/tests/rom/gb_test_runner path/to/test.gb --model dmg --max-cycles 300000000

# attach the real PPU (OAM DMA / LY / STAT / VRAM)
./build/tests/rom/gb_test_runner path/to/test.gb --ppu

# attach the APU (sound register/length/sweep tests)
./build/tests/rom/gb_test_runner path/to/test.gb --apu --model dmg

# render frames and compare against a reference PNG
./build/tests/rom/gb_test_runner path/to/acid2.gb --frames 10 \
    --compare-image path/to/reference.png

# patch ROM bytes before loading (repeatable, offset=hex)
./build/tests/rom/gb_test_runner path/to/test.gb --patch 0x4401=000000

# reproduce a model's post-boot CPU/DIV/I/O state without a boot ROM
./build/tests/rom/gb_test_runner path/to/boot_test.gb \
    --model dmg --boot-model dmgabc
```

Without `--ppu`, the CPU-only bus keeps a tiny LY/VBlank stub. With `--ppu`,
the real PPU handles OAM DMA, VRAM/OAM blocking, STAT/LYC and rendering.

## ROM setup

The ROMs are git submodules:

```text
third_party/gb-test-roms/       # Blargg, prebuilt ROMs
third_party/mooneye-test-roms/  # Mooneye, prebuilt ROMs
third_party/dmg-acid2/          # source + reference image
third_party/cgb-acid2/          # source + reference image
```

Initialize them after cloning:

```bash
git submodule update --init --recursive
```

The acid2 reference images are in the submodules, but the release ROMs are not
stored in git. Fetch them once (SHA-256 verified):

```bash
python3 tests/rom/fetch_acid2_roms.py
```

The default destination is `build/test-roms/acid2`; use
`-DQNEMU_ACID2_ROM_DIR=...` if the ROMs live elsewhere.

CMake registers a curated CPU/timing/PPU subset automatically when the
submodules are present, so plain `ctest` covers them:

```bash
ctest --preset debug -L rom
ctest --preset debug -L blargg
ctest --preset debug -L mooneye
ctest --preset debug -L ppu
ctest --preset debug -L sound
ctest --preset debug -L mbc
ctest --preset debug -L acid2
```

The acid2 tests render 10 frames and require an exact 160×144 match. Use
`-DQNEMU_ENABLE_ROM_TESTS=OFF` to disable the curated tests, or
`-DQNEMU_TEST_ROM_ROOT=...` if the submodules live elsewhere. For an additional
directory tree, `-DQNEMU_TEST_ROMS_DIR=...` registers every `.gb` below it.

To run a directory tree manually and get a summary:

```bash
python3 tests/rom/run_test_roms.py third_party/gb-test-roms \
    --include 'cpu_instrs|instr_timing|mem_timing'

python3 tests/rom/run_test_roms.py third_party/mooneye-test-roms/acceptance \
    --include 'ppu/' --ppu
```

## Curated test ROMs

The current set registers 164 ROM tests from the submodules plus 4 acid2
image tests, all passing.

<details>
<summary>Blargg CPU and timing tests (<code>-L blargg</code>)</summary>

- `cpu_instrs/individual/01-special` … `11-op a,(hl)`
- `cpu_instrs/cpu_instrs`
- `cpu_instrs/individual/01-special` on CGB hardware (boot ROM compatibility
  palette and KEY0 handoff)
- `instr_timing/instr_timing`
- `halt_bug`
- `interrupt_time` (CGB, requires `--apu` for the speed detector)
- `mem_timing/individual/01-read_timing`, `02-write_timing`, `03-modify_timing`
- `mem_timing/mem_timing`
- `mem_timing-2/rom_singles/01-read_timing`, `02-write_timing`, `03-modify_timing`
- `mem_timing-2/mem_timing`

</details>

<details>
<summary>Blargg sound tests (<code>-L sound</code>, requires <code>--apu</code>)</summary>

DMG (`--model dmg`):

- `dmg_sound/rom_singles/01-registers`
- `02-len ctr`
- `03-trigger`
- `04-sweep`
- `05-sweep details`
- `06-overflow on trigger`
- `07-len sweep period sync`
- `08-len ctr during power`
- `09-wave read while on`
- `10-wave trigger while on`
- `11-regs after power`
- `12-wave write while on`

CGB (`--model cgb`):

- `cgb_sound/rom_singles/01-registers`
- `02-len ctr`
- `03-trigger`
- `04-sweep`
- `05-sweep details`
- `06-overflow on trigger`
- `07-len sweep period sync`
- `08-len ctr during power`
- `09-wave read while on`
- `10-wave trigger while on`
- `11-regs after power`
- `12-wave`

</details>

<details>
<summary>Blargg OAM corruption tests (<code>-L ppu</code>, requires <code>--ppu</code>)</summary>

DMG (`--model dmg`):

- `oam_bug/rom_singles/1-lcd_sync`
- `2-causes`
- `3-non_causes`
- `4-scanline_timing`
- `5-timing_bug`
- `6-timing_no_bug`
- `7-timing_effect`
- `8-instr_effect`

`7-timing_effect` is run with a small ROM patch that disables the ROM's
optional `write_text_out` log.  The log grows past the 8 KiB cartridge
RAM (about 9.5 KiB of OAM dumps) and overwrites the test code copied to
`$C000`; it is redundant with the console output and is not part of the
checksum.  The OAM corruption and CRC behaviour are unchanged.

</details>

<details>
<summary>Mooneye MBC tests (<code>-L mbc</code>, real <code>GbCartridge</code>/MBC code)</summary>

- `emulator-only/mbc1/bits_bank1`, `bits_bank2`, `bits_mode`, `bits_ramg`
- `emulator-only/mbc1/multicart_rom_8Mb` (MBC1M)
- `emulator-only/mbc1/ram_64kb`, `ram_256kb`
- `emulator-only/mbc1/rom_1Mb`, `rom_2Mb`, `rom_4Mb`, `rom_8Mb`, `rom_16Mb`, `rom_512kb`
- `emulator-only/mbc2/bits_ramg`, `bits_romb`, `bits_unused`, `ram`
- `emulator-only/mbc2/rom_512kb`, `rom_1Mb`, `rom_2Mb`
- `emulator-only/mbc5/rom_512kb`, `rom_1Mb`, `rom_2Mb`, `rom_4Mb`, `rom_8Mb`, `rom_16Mb`, `rom_32Mb`, `rom_64Mb`

</details>

<details>
<summary>Mooneye CPU/interrupt/timer tests (<code>-L mooneye</code>, CPU-only bus)</summary>

- `bits/reg_f`
- `div_timing`
- `ei_sequence`, `ei_timing`
- `halt_ime0_ei`, `halt_ime0_nointr_timing`, `halt_ime1_timing`, `halt_ime1_timing2-GS`
- `if_ie_registers`
- `instr/daa`
- `interrupts/ie_push`
- `intr_timing`
- `pop_timing`
- `rapid_di_ei`
- `reti_intr_timing`
- `timer/div_write`
- `timer/rapid_toggle`
- `timer/tim00`, `timer/tim01`, `timer/tim10`, `timer/tim11`
- `timer/tim00_div_trigger`, `timer/tim01_div_trigger`, `timer/tim10_div_trigger`, `timer/tim11_div_trigger`
- `timer/tima_reload`, `timer/tima_write_reloading`, `timer/tma_write_reloading`

</details>

<details>
<summary>Mooneye boot-state and unused I/O tests (<code>-L boot</code>)</summary>

The runner reproduces the model-specific post-boot CPU registers, DIV phase,
serial clock phase and I/O register values without executing a copyrighted
boot ROM.  `--boot-model` selects `dmg0`, `dmgabc`, `mgb`, `sgb` or `sgb2`.

- `bits/unused_hwio-GS` (DMG, with `--apu`)
- `boot_div-dmg0`, `boot_div-dmgABCmgb`, `boot_div-S`, `boot_div2-S`
- `boot_hwio-dmg0`, `boot_hwio-dmgABCmgb`, `boot_hwio-S`
- `boot_regs-dmg0`, `boot_regs-dmgABC`, `boot_regs-mgb`, `boot_regs-sgb`, `boot_regs-sgb2`
- `serial/boot_sclk_align-dmgABCmgb`

The actual boot ROMs are exercised through additional runs with `--boot-rom`:
`misc/boot_regs-cgb` and `misc/boot_hwio-C` on CGB, `boot_regs-dmgABC` on DMG
and `boot_regs-sgb` / `boot_hwio-S` on SGB.

</details>

<details>
<summary>Mooneye PPU/OAM/bus-timing tests (<code>-L ppu</code>, requires <code>--ppu</code>)</summary>

- `add_sp_e_timing`
- `call_timing`, `call_timing2`, `call_cc_timing`, `call_cc_timing2`
- `di_timing-GS`
- `jp_timing`, `jp_cc_timing`
- `ld_hl_sp_e_timing`
- `push_timing`
- `ret_timing`, `ret_cc_timing`, `reti_timing`, `rst_timing`
- `bits/mem_oam`
- `oam_dma/basic`, `oam_dma/reg_read`, `oam_dma/sources-GS` (DMG)
- `oam_dma_start`, `oam_dma_restart`, `oam_dma_timing`
- `ppu/hblank_ly_scx_timing-GS`
- `ppu/intr_1_2_timing-GS`
- `ppu/intr_2_0_timing`
- `ppu/intr_2_mode0_timing`
- `ppu/intr_2_mode0_timing_sprites`
- `ppu/intr_2_mode3_timing`
- `ppu/intr_2_oam_ok_timing`
- `ppu/lcdon_timing-GS`, `ppu/lcdon_write_timing-GS` (DMG)
- `ppu/stat_irq_blocking`
- `ppu/stat_lyc_onoff`
- `ppu/vblank_stat_intr-GS`

</details>

<details>
<summary>Acid2 image tests (<code>-L acid2</code>)</summary>

- `dmg-acid2`: exact 160×144 match against `reference-dmg.png`
- `cgb-acid2`: exact 160×144 match against `reference.png`
- Both also run once through the real boot ROM (240 frames)

</details>

## Known test limitations

The curated CTest set registers only passing ROMs. The following ROMs are
deliberately not registered and fail when run directly:

- MBC7, MMM01, HuC3, Camera and TAMA5 are not implemented (see the README's
  cartridge table).
- MBC6 flash contents (downloaded minigames) are not persisted, and the Mobile
  Adapter GB serial device / HTTP server is not emulated.
- `boot_div-*` and `boot_hwio-dmgABCmgb` assert the official boot ROMs' exact
  cycle counts/DIV values, which the shipped reimplementations do not
  reproduce (see [`boot-roms.md`](boot-roms.md)).

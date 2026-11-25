# Architecture

The host layer and the console implementation are deliberately separate:

```text
qnemu/
├── main.cpp            QGuiApplication + QML engine setup
├── ui/                 Qt/QML host layer
│   ├── *.qml           library, game window, settings and shared components
│   ├── AppController.* QML-facing C++ controller
│   ├── AppSettings.*   persistent global/platform settings
│   ├── GameLibrary.*   ROM scan, metadata and library models
│   ├── RomDatabase.*   built-in No-Intro metadata index
│   ├── RomArchive.*    libarchive-backed zip/7z/rar reader
│   └── EmulatorItem.*  QQuickPaintedItem frame view and key mapping
├── display/            DisplayInterface and FrameBuffer headers
├── sound/              Qt Multimedia audio output (shared host service)
├── save/               battery-save and RTC persistence (shared service)
└── gb/                 Game Boy implementation
    ├── cpu/            SM83 core
    ├── interrupt/      IF/IE and interrupt priority
    ├── gpu/            PPU timing core, DMG/CGB video paths, FIFO, renderer
    ├── apu/            APU and four channels
    ├── timer/          DIV/TIMA timer
    ├── cartridge/      cartridge header, MBCs
    ├── memory/         DMG/CGB work RAM and high RAM
    ├── joypad/         FF00 joypad matrix
    ├── mmu/            DMG/CGB address decoding and CPU bus adapters
    ├── platform/       DMG/SGB/CGB platform assemblies
    ├── sgb/            SGB command receiver, colorizer and frame sink
    └── Gb.cpp          host-side system facade
```

Each hardware block is a separate Qt-free static library:

- `gb_cpu`
- `gb_interrupt`
- `gb_ppu`
- `gb_sgb`
- `gb_apu`
- `gb_timer`
- `gb_cartridge`
- `gb_memory`
- `gb_joypad`
- `gb_mmu`

`gb` is the complete GB system library. It links the device libraries
publicly and only the save service privately. The host display, audio sink and
optional input backend are injected by the executable, so `gb` itself does not
depend on Qt Quick, Qt Widgets or Multimedia. The QML executable links `gb`,
`sound` and the Qt Quick modules; the headless ROM runner links the device
libraries it needs without pulling in Qt Multimedia or the GUI.

Platform selection is separate from cartridge capability. `Gb` owns the
emulation thread, pause/save service and host interfaces, while
`GameBoyPlatform` owns an assembled device graph:

- `DmgPlatform` builds `DmgMmu`/`DmgCpuBus`/`DmgWorkRam` and the `DmgPpu`
  timing/video path.
- `SgbPlatform` extends `DmgPlatform` with `GbSgb` and installs
  `SgbColorizer`/`SgbFrameSink`. The PPU itself only knows `PpuColorizer` and
  `PpuFrameSink`, so `gb_ppu` no longer depends on `gb_sgb`.
- `CgbPlatform` builds `CgbMmu`/`CgbCpuBus`/`CgbWorkRam` and `CgbPpu`, keeping
  the CGB boot ROM's DMG compatibility path for grey cartridges.

The PPU is split into a shared timing core (`GbPpu`) and a `PpuVideo`
implementation: `DmgPpuVideo` owns the monochrome path, while `CgbPpuVideo`
owns VRAM banking, palette RAM, HDMA and DMG-compatibility rendering. The dot
clock, mode transitions, OAM DMA and interrupts remain shared, so the
timing-sensitive code is not duplicated.

## User interface

The application shell is QML and uses two top-level windows. `Main.qml` is the
library/home window; `GameWindow.qml` hosts the `EmulatorItem` and opens only
when a ROM is running. `AppController` owns the `Gb` instance and exposes only
a small property/action surface, while `EmulatorItem` implements
`DisplayInterface` and `InputInterface` and paints the frame (160×144, or
256×224 with an SGB border) through a `QQuickPaintedItem`. The Quick Controls
style is platform-selected: `macOS` on macOS, `FluentWinUI3` on Windows, `iOS`
on iOS, and `Fusion` as the portable fallback. Native Quick Controls styles
require a recent Qt (`macOS` in 6.7+, `FluentWinUI3` in 6.8+); older Qt
versions use Fusion. ROM selection uses the native `FileDialog`; the File/View
menus live in the `ApplicationWindow` menu bar, which maps to the system menu
bar on macOS and appears in-window on other platforms.

The library window has a fixed comfortable size and is not affected by the
game scale. The game window is resized to the 100/150/200% preset. Closing the
game window or returning to the library pauses the emulator; the paused frame
stays visible behind a **Paused** overlay. The settings window is positioned
once when it opens and stays put when the game window is resized.

`GameLibrary` scans the configured GB/GBC folders and reads cartridge headers
for title, publisher, region and platform. Platform classification uses the
CGB flag, not the file extension: CGB-only cartridges (`0xC0`) go to the GBC
library, while DMG and dual-mode (`0x80`) cartridges stay in the GB library so
they can be launched as DMG, SGB or CGB. ROMs opened from other folders are
stored as references, so the original collection is never copied or
reorganized.

When a DMG-only cartridge is launched on the GBC platform, the PPU keeps the
DMG tile/attribute path but maps the four shades through the CGB compatibility
palettes. A dual-mode cartridge launched on the GBC platform uses the native CGB
PPU path instead. The CGB boot ROM performs the per-game selection: Nintendo
license check, title checksum, duplicate checksums disambiguated by the fourth
title character, and the palette combination table. Holding a direction,
optionally with A/B, during the logo overrides it with one of the 16 manual
combinations.

Window backgrounds, controls and text follow the platform palette, so the UI
adapts to light and dark appearance automatically. The game viewport keeps a
black background so the 160×144 frame is not tinted by the theme.

## CPU

The SM83 core lives in `qnemu/gb/cpu` and is independent of Qt, the MMU and the
GUI thread:

- `GbCpu` executes one instruction per `step()` and talks to the rest of the
  machine through `CpuBus` (`read` / `write` / `tick`).
- Every memory access is one machine cycle, so instruction timing is expressed
  directly in the instruction implementation instead of a separate table.
- `Registers` is a portable plain struct; flags use the usual `Z N H C` byte,
  with no anonymous unions or endianness assumptions.
- Interrupts are handled at instruction boundaries: IME, the EI
  one-instruction delay, IF/IE priority, HALT, the HALT bug and STOP.
- `GbCpuBus` adapts the MMU's 1 T-cycle `step()` to the CPU's 4 T-cycle bus
  accesses. The emulation thread lives in `Gb`, not in the CPU.

Unit tests are in `tests/gb/cpu/GbCpuTest.cpp` and use a flat `TestBus`.

## PPU

The PPU is split into four Qt-free pieces under `qnemu/gb/gpu`:

- `GbPpu` owns the dot/line clock, LCDC/STAT/LY/LYC state, OAM DMA, HDMA,
  palette RAM and the frame boundary. Mode 3 is driven dot by dot by
  `PpuFifo`; fine-scroll startup, window activation and object fetch penalties
  extend the line as they occur.
- `PpuFifo` is a background/window pixel FIFO. It fetches eight-pixel rows,
  emits one pixel per dot, handles SCX discard and window restarts, and keeps
  the per-pixel background colour/priority data used for object compositing.
  Moving the LCD from off to on restarts line 0 in mode 0 and shortens that
  first line, matching the DMG `lcdon` timing tests.
- `PpuMemory` owns both VRAM banks, OAM and the CGB palette RAM.
- `PpuRenderer` is a stateless pixel/scanline renderer. It is the reference
  implementation used by unit tests and by `PpuFifo` for per-pixel background
  fetches and the final object-compositing pass.

STAT interrupts use an edge/blocking line rather than firing on every mode
change. On CGB, the mode-2 STAT source is pulsed on the final dot of line 143,
one dot before VBlank; DMG/SGB pulse it at the VBlank boundary instead.
`GbCpuBus` also implements CGB `KEY0`/`KEY1` and double-speed switching,
including halving the number of peripheral dots per CPU T-cycle. Object fetch
penalties follow the tile-wait rule from Pan Docs; sprites at the left edge
share one alignment cost derived from `OAM X & 7`.

The PPU never touches Qt or `GbCartridgeInterface`. External memory needed by
OAM DMA/HDMA is supplied through a callback, and finished frames are pushed to
`DisplayInterface` as a plain 160×144 `FrameBuffer` (or a 256×224
`SgbFrameBuffer` when the SGB border is active). The QML `EmulatorItem`
paints that buffer into the scene without blocking the emulation thread.

PPU tests live in `tests/gb/gpu/` and cover background/window addressing,
signed tile indexes, DMG/CGB palettes, sprite priority, OAM DMA/HDMA, mode
transitions, STAT/LYC behaviour, the pixel FIFO, LCD enable/disable timing and
full-frame acid2 comparison.

## APU

The APU implements all four hardware channels:

- **CH1**: pulse wave with duty, envelope, length timer and frequency sweep.
- **CH2**: pulse wave with duty, envelope and length timer.
- **CH3**: 32-sample 4-bit wave-table channel with 100/50/25% output levels.
- **CH4**: LFSR noise with 15-bit/7-bit modes, envelope and length timer.

A 512 Hz frame sequencer clocks the length timers at 256 Hz, the CH1 sweep at
128 Hz and the volume envelopes at 64 Hz. It is driven by the DIV-APU falling
edge, so writing DIV synchronises the APU exactly like the hardware (including
the power-on DIV glitch).

The mixer applies NR51 panning and NR50 master volume, models the DACs'
negative slope, high-pass filters the DC offset like the hardware's output
capacitor, and downsamples to 44100 Hz stereo signed 16-bit samples. A 20 ms
fade-in on DAC enable suppresses the power-on click, and a gentle second-order
10 kHz Butterworth low-pass softens the raw square waves. Qt Multimedia plays
the samples through a small ring buffer; when the buffer is full the emulation
thread blocks, which also acts as the speed limiter. Loading a ROM flushes the
audio buffer so the previous game cannot leak into the next one.

Unit tests are in `tests/gb/apu/`. The headless runner can attach the APU with
`--apu`; curated `dmg_sound` / `cgb_sound` tests run with
`ctest --preset debug -L sound`.

The emulation thread is paced to the Game Boy's ~59.7 fps frame rate. The audio
ring drops new samples if the sink stalls instead of blocking the emulator, so
a slow or missing audio device cannot freeze the game.

## Memory and MMU

`GbWorkRam` models the CGB's 8 × 4 KiB work RAM banks:

- Bank 0 is fixed at `C000-CFFF`.
- `SVBK` (`FF70`) selects bank 1–7 at `D000-DFFF`; writing 0 selects bank 1.
- `SVBK` reads back as `0xF8 | bank` and resets to `0xF8`.
- Echo RAM mirrors the selected banks at `E000-FDFF`.
- In DMG compatibility mode `FF70` reads `FF`, writes are ignored, and
  `D000-DFFF` is the fixed upper 4 KiB.

`GbHighRam` owns `FF80-FFFE`. `GbMmu` decodes the full address space and routes
requests to the cartridge, PPU, APU, timer, joypad, RAM and interrupt
controller. `GbCpuBus` adds the CGB `KEY0`/`KEY1` registers and speed switching.

The MMU and memory blocks have their own tests in `tests/gb/mmu/` and
`tests/gb/memory/`.

## Cartridges

| Mapper | Status | Notes |
| ------ | ------ | ----- |
| No MBC | ✅ | ROM only |
| MBC1   | ✅ | unit + Mooneye MBC tests |
| MBC1M  | ✅ | detected from the extra header in bank `$10` |
| MBC2   | ✅ | built-in 512×4 RAM |
| MBC3   | ✅ | RTC register/latch, 1 Hz tick |
| MBC30  | ✅ | MBC3 with 64 KiB SRAM / 4 MiB ROM |
| MBC5   | ✅ | 9-bit ROM bank, 16 RAM banks, rumble bit |
| MBC6   | ✅ | Net de Get; dual ROM/flash windows, RAM banking, flash command set |
| HuC1   | ✅ | ROM/RAM/battery; IR port not connected |
| MBC7   | ❌ | not implemented |
| MMM01  | ❌ | not implemented |
| HuC3   | ❌ | not implemented |
| Camera / TAMA5 | ❌ | not implemented |

MBC3's RTC protocol and persistence via a timestamped `.rtc` file are
implemented. MBC30 is selected for MBC3 headers with 64 KiB SRAM (or a 4 MiB
ROM). HuC1 reports "no light" from its IR register because no IR backend is
connected yet. MBC6 models the MX29F008 flash command set (ID, status,
128-byte page program, sector/chip erase, sector 0 protection and the hidden
256-byte map) in memory; Net de Get boots to its title screen, but the flash
is not persisted yet and the Mobile Adapter download path is not emulated.

## Saves

Battery-backed cartridges are saved as raw cartridge RAM, without interpreting
the game's save format:

```text
<save folder>/<ROM stem>.sav   raw SRAM
<save folder>/<ROM stem>.rtc   MBC3 RTC registers + timestamp
```

`.sav` files are written for battery-backed MBC1/MBC2/MBC3/MBC5/MBC6, MBC30 and
no-MBC carts. MBC3 RTC carts additionally get an `.rtc` file; when the ROM is
loaded again, elapsed wall-clock time is applied unless the RTC was halted.
Saves are written when a ROM is switched or the emulator closes. The save
folder is configured in **Settings → General**.

## Input

Host input is separate from rendering. `EmulatorItem` implements
`InputInterface` and maps Qt keys to `GbButton` values; `GbJoypad` only models
the `FF00/P1` matrix and its edge-triggered interrupt. The joypad is therefore
Qt-free, and a future gamepad, replay or network backend can feed it without
touching the renderer.

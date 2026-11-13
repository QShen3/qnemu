# Using QNEmu

## Command line

```bash
./build/qnemu/qnemu
./build/qnemu/qnemu --launch-platform gbc   # override the platform for this run
./build/qnemu/qnemu --launch-platform gb
./build/qnemu/qnemu --launch-platform sgb
./build/qnemu/qnemu --rom path/to/game.gb
```

`-p` is accepted as a shorthand for `--launch-platform`. `--rom` loads a ROM on
startup; without it, use **File → Open...**.

For link play, `--rom game1.gb --link-rom game2.gb` starts a two-player link
session directly. Repeat `--link-rom` up to three times for a three- or
four-player session. `--rom game1.gbc --ir-rom game2.gbc` starts a two-player
Game Boy Color infrared session (which cannot be combined with `--link-rom`).
See [`serial-link.md`](serial-link.md) for the transport details.

Link play can also be started from the UI: the library header and each game's
`⋯` menu offer **Link**. Select two or four games (or **Same game** for all
players) and the game window opens with the corresponding number of screens.
Click a screen to give it keyboard focus; Space or Ctrl+P pauses all players.
Two-player sessions use a direct link cable; four-player sessions emulate the
DMG-07 adapter. The Link banner also offers **Infrared**, which forces two
players and launches both ROMs on the CGB platform; a DMG-only ROM is rejected.

Before starting, a setup dialog chooses the save for each player. The default
**Game save** is the file used in single-player, so Pokémon trades and other
linked progress carry straight back into normal play. **Link save 1/2/3** are
persistent separate slots for games where several players share the same ROM;
the dialog warns instead of letting two running games share one save file.


`--model dmg|sgb|cgb` remains as a deprecated alias for `--launch-platform`.

## Launch platform

The launch platform overrides the per-cartridge-type preferences for the
current run. When it is omitted, DMG-only and dual-mode cartridges use their
own **Startup platform** setting in General, while GBC-only cartridges always
start on **Game Boy Color**:

- On the **GBC** platform, a dual-mode (`0x80`) or CGB-only (`0xC0`) cartridge
  runs in native CGB mode, while a DMG-only cartridge runs in DMG compatibility
  mode with the CGB compatibility palette, so monochrome games are colorized.
- The **GB** platform forces the DMG CPU/PPU path.
- **SGB** keeps the DMG CPU/PPU path but enables the Super Game Boy command
  protocol, palette and attribute commands, the default SGB border and custom
  border transfers (`CHR_TRN`/`PCT_TRN`/`ATTR_TRN`/`PAL_TRN`).

Cartridge type is derived from the header, not the file extension: `0xC0` is
CGB-only, `0x80` is dual-mode, and anything else is DMG-only.

## The library and the game window

The library and the game view are separate windows. The library window keeps a
comfortable fixed layout independent of the game scale; the game window opens
at the configured scale and can be resized. Closing the game window pauses
emulation; click the same library row again to resume it, or use
**Game → Resume** (Ctrl+P) or Space from the game window.

The home page has two library tabs (cartridge type, not launch platform):

- **Open ROM...** plays a ROM immediately in the game window. ROMs outside the
  configured library folders are added to the library as references; the
  original file is never copied or moved.
- **Add Game...** adds a ROM to the library without starting it.
- Click a library row to run it. Clicking the already-loaded game resumes it
  instead of reloading; the row shows a **Running** or **Paused** badge. The row
  menu can remove it from the library without deleting the file.
- **Settings...** opens the settings window (see below).
- **Screenshot** saves the current frame (160×144, or 256×224 with the SGB
  border) using the configured output folder and format.
- **View → Library / Game** raises the library or game window.
- **Game → Pause / Resume** (Ctrl+P) pauses or resumes the running game.

The library page uses a compact header, a platform switcher with live counts
and rounded game cards with a platform badge, publisher/region line and a hover
action menu; an empty library shows a cartridge illustration and shortcuts to
add a game or open the settings.

## Settings

The settings window is a sidebar layout with grouped cards:

- **General**: game-window scale (100/150/200%), audio enable, the startup
  platform for grey and dual-mode cartridges, save folder, screenshot folder
  and format (the format list comes from Qt's image writers). The library
  window is not affected by the scale setting.
- **Game Boy**: the ROM folder scanned for the GB library (grey and dual-mode
  cartridges), with a live count and a rescan button.
- **Game Boy Color**: the GBC-only ROM folder, with a live count and a rescan
  button. GBC-only cartridges always start on Game Boy Color, so there is no
  per-type preference for them.
- **Controls**: named key profiles, profile creation/deletion, and
  click-to-capture key bindings. Changes apply immediately.

All paths, scale, audio, screenshot settings and key profiles persist through
`QSettings`; key bindings apply to the game view immediately. Library paths are
per platform; the save path and key profiles are global for now.

### Default key bindings

Each profile can be changed in **Settings → Controls**:

| Joypad | Key |
| ------ | --- |
| Up     | W   |
| Down   | S   |
| Left   | A   |
| Right  | D   |
| A      | J   |
| B      | K   |
| Start  | 3   |
| Select | 2   |

## Game database

The library ships a No-Intro snapshot for Game Boy and Game Boy Color. It
replaces the cartridge header title with the canonical database title, adds
developer/publisher/region/languages/release information, and shows an
**Official** badge when a ROM's SHA-1 matches a licensed release. Known
prototype, aftermarket, unlicensed, pirate and homebrew dumps are still
identified, but use their own badge instead of "Official". A ROM that is not
in the database keeps the header metadata and has no badge.

The snapshot is built into the executable, so matching needs no network access.
Open a row's **⋯ → Game information** menu to see the database name, all
metadata, CRC32/SHA-1 and the source/license. See
[`rom-database.md`](rom-database.md) for sources, status rules, language
inference and the regeneration workflow.

## ROM archives

The library also scans `.zip`, `.7z` and `.rar` files through libarchive. An
archive must contain exactly one `.gb`, `.gbc` or `.sgb` entry; it is read into
memory, never extracted to disk. The inner ROM is hashed and matched against
the built-in database, so archives get the same Official badges and metadata as
plain ROMs. Opening an archive uses the archive filename for save files, so
`Tetris (World).zip` still produces `Tetris (World).sav`.

Encrypted, corrupt, oversized and multi-ROM archives are rejected instead of
being guessed. The library row stays unverified, and opening the archive shows
the reason in the status toast. The first scan runs in parallel on Qt's thread
pool and keeps the library visible with a loading overlay; later scans in the
same session use an in-memory cache.

## Searching the library

The search field covers title, publisher, developer, serial, genre, checksum,
platform and archive entry. Region, language and status have dedicated
dropdown filters. Search terms are case-insensitive and space-separated; every
term must match, and all filters combine. The tab counts and the empty state
follow the active filters.

## Save data

Each game's `⋯` menu has **Save data...**: play any slot in single-player,
rename the four slots, copy between slots, create a timestamped backup,
restore or delete backups, import/export a `.sav` (with its sibling `.rtc` when
present), or delete a slot. Backups go to the `backups` folder inside the save
directory and can be restored from the manager.

How saves are stored on disk is described in
[`architecture.md`](architecture.md#saves).

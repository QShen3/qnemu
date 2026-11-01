# Serial link

QNEmu implements the Game Boy serial port (`FF01` SB / `FF02` SC), a
two-player link cable and a four-player DMG-07 adapter.  This is the foundation
for local multiplayer; a socket transport for network play can be added behind
the same interface.

## Registers and timing

| Register | Behaviour |
| --- | --- |
| `FF01` SB | Shift register.  Writes are published to the peer immediately. |
| `FF02` SC | Bit 7 starts a transfer, bit 1 selects the CGB fast clock, bit 0 selects internal or external clock.  Bits 6-2 read as 1. |

The transfer takes eight bit periods:

| Clock | Normal speed | CGB double speed |
| --- | --- | --- |
| 8192 Hz | 512 T-cycles/bit | 256 T-cycles/bit |
| 262144 Hz | 16 T-cycles/bit | 8 T-cycles/bit |

`GbSerial` is advanced once per peripheral cycle by the MMU.  A transfer with
the internal clock completes after eight periods, clears SC bit 7 and requests
the serial interrupt.  With the external clock it waits until the peer clocks
the byte.  An internally clocked transfer without a link completes with
`0xFF`, matching a disconnected cable.

## Link transport

`GbSerialLink` is the per-endpoint interface.  The clock owner calls
`beginTransfer()` (which captures the peer's current SB) and later
`finishTransfer()` (which delivers its own byte).  The other endpoint polls
`hasPendingByte()` and consumes the result with `takeReceivedByte()`.  This
keeps the serial device independent of the transport.

`GbLocalSerialLink` implements the interface for two endpoints in one process.
The endpoints share a small mutex-protected state block and an atomic pending
flag, so the two emulation threads can poll and exchange bytes without stalling.
If both sides select the internal clock, the first endpoint owns the byte and
the second behaves as a slave for that transfer.

## Four-player adapter

`GbFourPlayerAdapter` implements the DMG-07 protocol for 3 or 4 players.  The
adapter, not a Game Boy, owns the serial clock; all Game Boys use the external
clock.  Player 1's emulation thread advances the adapter schedule.

- **Ping phase:** four-byte packets (`FE`, three STAT bytes) report the
  connected-player mask and each port's player id.  Players reply with ACK,
  RATE and SIZE.  Four consecutive `AA` replies from a player request the
  transmission phase.
- **Transmission phase:** the adapter sends the previous packet's aggregate
  (`SIZE × 4` bytes, player by player) while collecting the new bytes.  Missing
  players contribute zeroes.  `SIZE` is 1-4; the packet interval follows the
  RATE setting.
- **Restart:** three or more `FF` replies switch the adapter back to the ping
  phase through a full `FF` indicator packet.

The adapter waits for every attached game to arm its serial port before each
byte, so a slow ISR stretches the packet instead of dropping data.  The fixed
byte and packet timings come from Pan Docs and are rounded to peripheral
T-cycles; this is enough for games that use the adapter as a byte transport.

## UI

- The library header has a **Link** button; a row's `⋯` menu also has
  **Link Play...**.  Choose 2 or 4 players, then select Player 1 through
  Player N, or use **Same game** for all players.  `LinkWindow.qml` lays out
  two or four `EmulatorItem`s; click a screen to give it keyboard focus.
- A setup dialog shows which save each player will use.  **Game save** is the
  normal single-player save, so progress made while linked carries over.
  **Link save 1/2/3** are persistent per-ROM slots for cases where several
  players use the same ROM.
- For a Pokémon trade between Red and Blue, leave both players on **Game save**:
  the trade is written back to `Red.sav` and `Blue.sav`.  For multiple copies
  of the same ROM, give each additional player a link save.
- A row's `⋯` menu also has **Save data...**, a save manager for that ROM:
  play any slot in single-player, rename the four slots, copy between slots,
  create a backup, restore or delete backups, import or export a `.sav`, or
  delete a slot.  When a save is imported or exported, a sibling `.rtc` file
  (if present) is carried along.  Backups are written to the `backups` folder
  inside the save directory and listed under **Backups** in the manager.  To
  keep a link-save playthrough as the normal save, copy it to **Game save**.
- `--rom A --link-rom B` starts two-player link play directly.  Repeat
  `--link-rom` up to three times for three or four players; save slots are
  assigned automatically to avoid sharing a file.
- Space / Ctrl+P pauses all sessions, and Back to Library destroys them after
  flushing their saves.

## Tests

- `tests/gb/serial/GbSerialTest.cpp` covers register masks, both clock rates,
  CGB double speed, abort/reset, no-cable `0xFF`, master/slave exchange and the
  both-master fallback.
- `tests/gb/serial/GbLinkTest.cpp` runs two complete `Gb` systems on separate
  threads with a real link.  Each side runs a small MBC1 test ROM, exchanges a
  byte, and writes its result to cartridge RAM; the test reads the two `.sav`
  files after shutdown.
- `tests/gb/serial/GbFourPlayerAdapterTest.cpp` drives four fake endpoints
  through ping, connection reporting, the `AA`/`CC` switch, transmission
  packets and the `FF` restart path.
- `tests/gb/serial/GbFourPlayerLinkTest.cpp` runs four complete `Gb` systems
  with the adapter and `tests/rom/four_player/four_player_test.gb`.  The ROM
  pings, enters transmission mode, sends each player's id and checks the next
  broadcast packet for `1,2,3,4`; each system writes its result to SRAM.
- `tests/ui/SavePathsTest.cpp` checks that the path shown in the link setup
  dialog is the same path `SaveManager` will actually read and write, and
  covers copying saves with and without a sibling `.rtc`.

## Known limitations

- The four-player adapter supports up to four connected Game Boys; chaining
  adapters and the 16-player FaceBall cable are not implemented.
- The adapter's packet timing is rounded from Pan Docs.  Games that depend on
  exact DMG-07 microsecond timing may need further calibration.
- SB writes during an active two-player transfer are not blended into the
  outgoing byte; games normally write SB before starting the transfer.
- The start of an internally clocked transfer is not phase-aligned to DIV, so
  boot-time serial clock alignment is not reproduced.  This does not affect
  link transfers, where the clock owner defines the pacing.
- An external-clock transfer without a clock source waits forever, as on real
  hardware; games are expected to use their own timeout.

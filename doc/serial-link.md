# Serial link

QNEmu implements the Game Boy serial port (`FF01` SB / `FF02` SC) and an
in-process link cable.  This is the foundation for local two-player games; a
socket transport for network play can be added behind the same interface.

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

## UI

- The library header has a **Link** button; a row's `⋯` menu also has
  **Link Play...**.  Select Player 1 followed by Player 2, or use **Same game**
  twice.  This starts `LinkWindow.qml` with two `EmulatorItem`s side by side;
  click a screen to give it keyboard focus.
- A setup dialog shows which save each player will use.  **Game save** is the
  normal single-player save, so progress made while linked carries over.
  **Link save 1/2** are persistent per-ROM slots for a second player when both
  players use the same ROM.
- For a Pokémon trade between Red and Blue, leave both players on **Game save**:
  the trade is written back to `Red.sav` and `Blue.sav`.  For two copies of the
  same ROM, give Player 2 a link save and trade through that separate save.
- A row's `⋯` menu also has **Save data...**, a save manager for that ROM:
  play any slot in single-player, rename the three slots, copy between slots,
  create a backup, restore or delete backups, import or export a `.sav`, or
  delete a slot.  When a save is imported or exported, a sibling `.rtc` file
  (if present) is carried along.  Backups are written to the `backups` folder
  inside the save directory and listed under **Backups** in the manager.  To
  keep a link-save playthrough as the normal save, copy it to **Game save**.
- `--rom A --link-rom B` starts link play directly.  It uses normal saves, and
  falls back to Link save 2 for Player 2 when the two ROMs resolve to one save.
- Space / Ctrl+P pauses both sessions, and Back to Library destroys them after
  flushing their saves.

## Tests

- `tests/gb/serial/GbSerialTest.cpp` covers register masks, both clock rates,
  CGB double speed, abort/reset, no-cable `0xFF`, master/slave exchange and the
  both-master fallback.
- `tests/gb/serial/GbLinkTest.cpp` runs two complete `Gb` systems on separate
  threads with a real link.  Each side runs a small MBC1 test ROM, exchanges a
  byte, and writes its result to cartridge RAM; the test reads the two `.sav`
  files after shutdown.
- `tests/ui/SavePathsTest.cpp` checks that the path shown in the link setup
  dialog is the same path `SaveManager` will actually read and write, and
  covers copying saves with and without a sibling `.rtc`.

## Known limitations

- Only two endpoints are supported (no four-player hub).
- SB writes during an active transfer are not blended into the outgoing byte;
  games normally write SB before starting the transfer.
- The start of an internally clocked transfer is not phase-aligned to DIV, so
  boot-time serial clock alignment is not reproduced.  This does not affect
  link transfers, where the clock owner defines the pacing.
- An external-clock transfer without a clock source waits forever, as on real
  hardware; games are expected to use their own timeout.

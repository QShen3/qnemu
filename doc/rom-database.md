# Built-in game database

The library ships a metadata snapshot for Game Boy and Game Boy Color ROMs.
It is used to turn a cartridge header into a proper title, publisher, region,
languages and release information, and to mark known clean dumps.

## Sources

| Field | Source |
| --- | --- |
| Canonical name, region, CRC32/MD5/SHA-1, dump status | [No-Intro](https://no-intro.org/) via [libretro-database](https://github.com/libretro/libretro-database) |
| Developer, publisher, genre, release year, serial | libretro-database's `developer`, `publisher`, `genre`, `releaseyear` and `serial` DATs |

The snapshot is pinned to libretro-database commit
`073c9d3fadb8c15d0145c608dd102b67a7c82407` and DAT version `2026.10.07`.
It contains 4,546 entries, of which 3,149 are marked as official releases.

No-Intro is the authority for "is this a known, verified dump?".  The
developer/publisher DATs are best-effort: when they have no entry, the library
falls back to the publisher encoded in the cartridge header.

## Official vs. verified

A checksum match only means the file is a known clean dump; it does not prove
where the file came from.  The UI distinguishes the two:

- **Official** — a licensed release.  The No-Intro file name has none of the
  `(Unl)`, `(Aftermarket)`, `(Pirate)`, `(Proto)`, `(Beta)`, `(Sample)`,
  `(Homebrew)` or `(Demo)` tags.
- **Aftermarket / Unlicensed / Prototype / Pirate / Homebrew / Demo** — a known
  dump, but not an original licensed release.  Its metadata is still shown.
- **No badge** — the file is not in the database.  The library uses the
  cartridge header and marks it as unknown.

The badge is intentionally not called "verified" for non-official entries:
matching a prototype or an unlicensed dump is still a valid database match,
just not an official release.

## Languages

No-Intro only writes a language tag when a release contains more than one
language, for example `(Europe) (En,Fr,De,Es,It)`.  For untagged releases the
database infers the primary language from the release region (Japan → Japanese,
USA/Europe → English, France → French, and so on).  Inferred values are flagged
in the game-information dialog with "(inferred)".  This follows the No-Intro
convention rather than guessing from the ROM contents.

## Checksums

Entries are keyed by SHA-1.  The library computes SHA-1 and CRC32 in one pass
while scanning.  CRC32 and size are shown for reference; SHA-1 decides whether
a file matches.  MD5 is stored as well, matching the No-Intro DAT.

For `.zip`, `.7z` and `.rar` files, libarchive extracts the single ROM entry
into memory first; the checksum is computed over the decompressed ROM bytes,
not over the archive.  This is why an archived ROM can match the same database
entry as a plain ROM.

## Updating the snapshot

The generated file is committed so normal builds are offline and reproducible.
To refresh it:

1. Pick a new libretro-database commit and update `SOURCE_COMMIT` in
   `tools/build_rom_database.py`.
2. Run:

   ```bash
   python3 tools/build_rom_database.py
   ```

   The script downloads the DATs for that commit and writes
   `qnemu/data/romdb.json`.
3. Review the diff (entry counts, source metadata and a few known games).
4. Run the tests:

   ```bash
   cmake --build --preset debug
   ctest --preset debug -R 'RomDatabase|GameLibrary'
   ```

For an offline run, download the DATs into a directory named
`<category>-Game Boy.dat` / `<category>-Game Boy Color.dat` and pass
`--dat-dir <directory>`.

## License

The metadata snapshot is derived from libretro-database, which is licensed
under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/), and
from No-Intro's data.  The generated `qnemu/data/romdb.json` is a separate data
work under CC BY-SA 4.0; qnemu's source code remains MIT-licensed.  The
game-information dialog shows the source and license, and this file provides
the attribution required by the data license.

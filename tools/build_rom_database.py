#!/usr/bin/env python3
"""Build qnemu's built-in Game Boy / Game Boy Color metadata database.

The database is a snapshot of the libretro-database copy of No-Intro DATs.
No-Intro supplies the verified dump checksums, canonical names and regions;
libretro's developer/publisher/genre/release-year/serial DATs fill in the
remaining fields.  The output is committed so normal builds stay offline and
reproducible.

Usage:
    python3 tools/build_rom_database.py
    python3 tools/build_rom_database.py --dat-dir /path/to/dats
    python3 tools/build_rom_database.py --output /tmp/romdb.json

The pinned commit is intentional: updating the database should be a reviewed
change, not something that silently changes between builds.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import pathlib
import re
import sys
import tempfile
import time
import urllib.parse
import urllib.request
from collections import Counter
from typing import Iterable

SOURCE_COMMIT = "073c9d3fadb8c15d0145c608dd102b67a7c82407"
SOURCE_URL = "https://github.com/libretro/libretro-database"
SOURCE_LICENSE = "CC BY-SA 4.0"
SOURCE_LICENSE_URL = "https://creativecommons.org/licenses/by-sa/4.0/"
SOURCE_NAME = "libretro-database (No-Intro)"

SYSTEMS = {
    "GB": ("Game Boy", "Nintendo - Game Boy"),
    "GBC": ("Game Boy Color", "Nintendo - Game Boy Color"),
}

# No-Intro's language codes, in the order mandated by its convention.
LANGUAGE_ORDER = ["En", "Ja", "Fr", "De", "Es", "It", "Nl", "Pt", "Sv", "No", "Da", "Fi", "Zh", "Ko", "Pl", "Ru"]
LANGUAGE_NAMES = {
    "En": "English",
    "Ja": "Japanese",
    "Fr": "French",
    "De": "German",
    "Es": "Spanish",
    "It": "Italian",
    "Nl": "Dutch",
    "Pt": "Portuguese",
    "Sv": "Swedish",
    "No": "Norwegian",
    "Da": "Danish",
    "Fi": "Finnish",
    "Zh": "Chinese",
    "Ko": "Korean",
    "Pl": "Polish",
    "Ru": "Russian",
}

REGIONS = {
    "Asia",
    "Australia",
    "Brazil",
    "Canada",
    "China",
    "Denmark",
    "Europe",
    "Finland",
    "France",
    "Germany",
    "Hong Kong",
    "Italy",
    "Japan",
    "Korea",
    "Netherlands",
    "New Zealand",
    "Norway",
    "Poland",
    "Portugal",
    "Russia",
    "Spain",
    "Sweden",
    "Taiwan",
    "United Kingdom",
    "USA",
    "Unknown",
    "World",
}

# No-Intro only writes the language tag when a release contains more than one
# language.  For untagged releases the primary language follows the region.
REGION_LANGUAGES = {
    "Asia": [],
    "Australia": ["En"],
    "Brazil": ["Pt"],
    "Canada": ["En"],
    "China": ["Zh"],
    "Denmark": ["Da"],
    "Europe": ["En"],
    "Finland": ["Fi"],
    "France": ["Fr"],
    "Germany": ["De"],
    "Hong Kong": ["Zh"],
    "Italy": ["It"],
    "Japan": ["Ja"],
    "Korea": ["Ko"],
    "Netherlands": ["Nl"],
    "New Zealand": ["En"],
    "Norway": ["No"],
    "Poland": ["Pl"],
    "Portugal": ["Pt"],
    "Russia": ["Ru"],
    "Spain": ["Es"],
    "Sweden": ["Sv"],
    "Taiwan": ["Zh"],
    "United Kingdom": ["En"],
    "USA": ["En"],
    "Unknown": [],
    "World": ["En"],
}

STATUS_PRIORITY = {
    "official": 0,
    "aftermarket": 1,
    "unlicensed": 2,
    "proto": 3,
    "homebrew": 4,
    "demo": 5,
    "pirate": 6,
}

GAME_BLOCK = re.compile(r"^game \(\n(.*?)^\)\n", re.MULTILINE | re.DOTALL)
NAME_LINE = re.compile(r'^\tname "(.*)"$', re.MULTILINE)
REGION_LINE = re.compile(r'^\tregion "(.*)"$', re.MULTILINE)
FIELD_LINE = re.compile(r'^\t(developer|publisher|genre|releaseyear|serial) "(.*)"$', re.MULTILINE)
ROM_LINE = re.compile(
    r'rom \( name "(.*?)" size (\d+) crc ([0-9A-Fa-f]{8}) '
    r"md5 ([0-9A-Fa-f]{32}) sha1 ([0-9A-Fa-f]{40})[^)]*\)"
)
ROM_CRC_LINE = re.compile(r"rom \( crc ([0-9A-Fa-f]{8}) \)")


def fetch(url: str) -> bytes:
    last_error: Exception | None = None
    for attempt in range(3):
        print(f"  GET {url} (attempt {attempt + 1}/3)", file=sys.stderr)
        try:
            request = urllib.request.Request(
                url, headers={"User-Agent": "qnemu-romdb-builder/1"}
            )
            with urllib.request.urlopen(request, timeout=120) as response:
                return response.read()
        except (OSError, TimeoutError) as error:
            last_error = error
            if attempt < 2:
                time.sleep(1 + attempt)
    raise SystemExit(f"failed to fetch {url}: {last_error}")


def dat_url(system: str, category: str) -> str:
    quoted = urllib.parse.quote(f"{system}.dat")
    return (
        f"https://raw.githubusercontent.com/libretro/libretro-database/"
        f"{SOURCE_COMMIT}/metadat/{category}/{quoted}"
    )


def load_dat(
    dat_dir: pathlib.Path | None, category: str, short_name: str, system: str
) -> str:
    if dat_dir is not None:
        path = dat_dir / f"{category}-{short_name}.dat"
        if not path.exists():
            raise SystemExit(f"missing DAT: {path}")
        return path.read_text(encoding="utf-8")
    return fetch(dat_url(system, category)).decode("utf-8")


def parse_blocks(text: str) -> Iterable[str]:
    return GAME_BLOCK.findall(text)


def first_match(pattern: re.Pattern[str], text: str, default: str = "") -> str:
    match = pattern.search(text)
    return match.group(1) if match else default


def parse_rom_entries(text: str) -> list[dict[str, str]]:
    entries: list[dict[str, str]] = []
    for block in parse_blocks(text):
        rom = ROM_LINE.search(block)
        if not rom:
            continue
        name = first_match(NAME_LINE, block)
        if not name:
            continue
        entries.append(
            {
                "name": name,
                "region": first_match(REGION_LINE, block),
                "romName": rom.group(1),
                "size": rom.group(2),
                "crc": rom.group(3).upper(),
                "md5": rom.group(4).upper(),
                "sha1": rom.group(5).upper(),
            }
        )
    return entries


def parse_crc_metadata(text: str) -> dict[str, str]:
    result: dict[str, str] = {}
    for block in parse_blocks(text):
        rom = ROM_CRC_LINE.search(block)
        field = FIELD_LINE.search(block)
        if not rom or not field:
            continue
        result.setdefault(rom.group(1).upper(), field.group(2))
    return result


def dat_version(text: str) -> str:
    match = re.search(r'^\tversion "(.*)"$', text, re.MULTILINE)
    return match.group(1) if match else ""


def paren_groups(name: str) -> list[str]:
    return re.findall(r"\(([^()]*)\)", name)


def split_tags(group: str) -> list[str]:
    return [part.strip() for part in group.split(",")]


def region_tags(name: str) -> list[str]:
    for group in paren_groups(name):
        parts = split_tags(group)
        if parts and all(part in REGIONS for part in parts):
            return parts
    return []


def language_tags(name: str) -> list[str]:
    for group in paren_groups(name):
        parts = split_tags(group)
        if parts and all(part in LANGUAGE_NAMES for part in parts):
            return parts
    return []


def clean_title(name: str) -> str:
    """Drop the No-Intro trailing tags while keeping parentheses in the title."""
    for match in re.finditer(r"\(([^()]*)\)", name):
        parts = split_tags(match.group(1))
        if not parts:
            continue
        if all(part in REGIONS for part in parts) or all(part in LANGUAGE_NAMES for part in parts):
            return name[: match.start()].rstrip()
        if parts[0].startswith(("Rev ", "v", "Proto", "Beta", "Sample", "Unl", "Aftermarket", "Pirate")):
            return name[: match.start()].rstrip()
    return name.strip()


def status_for(rom_name: str) -> str:
    tags = [tag for group in paren_groups(rom_name) for tag in split_tags(group)]
    if any("Pirate" in tag for tag in tags):
        return "pirate"
    if any("Aftermarket" in tag for tag in tags):
        return "aftermarket"
    if any(tag == "Unl" for tag in tags):
        return "unlicensed"
    if any(("Proto" in tag or "Beta" in tag or "Sample" in tag) for tag in tags):
        return "proto"
    if any("Homebrew" in tag for tag in tags):
        return "homebrew"
    if any(tag == "Demo" for tag in tags):
        return "demo"
    return "official"


def languages_for(name: str, region: str) -> tuple[list[str], bool]:
    explicit = language_tags(name)
    if explicit:
        return explicit, False
    regions = region_tags(name)
    if not regions and region:
        regions = split_tags(region)
    languages: list[str] = []
    for item in regions:
        for language in REGION_LANGUAGES.get(item, []):
            if language not in languages:
                languages.append(language)
    languages.sort(key=LANGUAGE_ORDER.index)
    return languages, bool(languages)


def release_year(value: str) -> int:
    match = re.search(r"\b(19\d{2}|20\d{2})\b", value)
    return int(match.group(1)) if match else 0


def build(dat_dir: pathlib.Path | None) -> dict:
    systems: dict[str, dict] = {}
    for platform, (short_name, system) in SYSTEMS.items():
        no_intro = load_dat(dat_dir, "no-intro", short_name, system)
        systems[platform] = {
            "version": dat_version(no_intro),
            "entries": parse_rom_entries(no_intro),
            "developer": parse_crc_metadata(load_dat(dat_dir, "developer", short_name, system)),
            "publisher": parse_crc_metadata(load_dat(dat_dir, "publisher", short_name, system)),
            "genre": parse_crc_metadata(load_dat(dat_dir, "genre", short_name, system)),
            "releaseyear": parse_crc_metadata(load_dat(dat_dir, "releaseyear", short_name, system)),
            "serial": parse_crc_metadata(load_dat(dat_dir, "serial", short_name, system)),
        }

    records: dict[str, dict] = {}
    stats = Counter()
    for platform, data in systems.items():
        for entry in data["entries"]:
            crc = entry["crc"]
            languages, inferred = languages_for(entry["name"], entry["region"])
            regions = region_tags(entry["name"]) or split_tags(entry["region"])
            record = {
                "title": clean_title(entry["name"]),
                "name": entry["name"],
                "platform": platform,
                "region": ", ".join(regions),
                "languages": languages,
                "languagesInferred": inferred,
                "developer": data["developer"].get(crc, ""),
                "publisher": data["publisher"].get(crc, ""),
                "genre": data["genre"].get(crc, ""),
                "year": release_year(data["releaseyear"].get(crc, "")),
                "serial": data["serial"].get(crc, ""),
                "size": int(entry["size"]),
                "crc32": crc,
                "md5": entry["md5"],
                "sha1": entry["sha1"],
                "status": status_for(entry["romName"]),
            }
            existing = records.get(entry["sha1"])
            if existing is None or STATUS_PRIORITY[record["status"]] < STATUS_PRIORITY[existing["status"]]:
                records[entry["sha1"]] = record
            if existing is not None:
                stats["duplicates"] += 1
            stats[record["status"]] += 1
            stats[f"platform:{platform}"] += 1
            if not record["publisher"]:
                stats["missing_publisher"] += 1
            if not record["developer"]:
                stats["missing_developer"] += 1
            if not record["languages"]:
                stats["missing_languages"] += 1

    ordered = sorted(
        records.values(),
        key=lambda item: (item["platform"], item["title"].casefold(), item["region"], item["sha1"]),
    )
    versions = {platform: data["version"] for platform, data in systems.items()}
    print(f"entries: {len(ordered)} (from {sum(len(data['entries']) for data in systems.values())} DAT rows)", file=sys.stderr)
    print(f"versions: {versions}", file=sys.stderr)
    print(f"stats: {dict(stats)}", file=sys.stderr)

    return {
        "schema": 1,
        "source": SOURCE_NAME,
        "sourceUrl": SOURCE_URL,
        "sourceCommit": SOURCE_COMMIT,
        "sourceLicense": SOURCE_LICENSE,
        "sourceLicenseUrl": SOURCE_LICENSE_URL,
        "datVersions": versions,
        "generated": _dt.date.today().isoformat(),
        "entries": ordered,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dat-dir",
        type=pathlib.Path,
        help="read <category>-<system>.dat files from this directory instead of downloading",
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parents[1] / "qnemu" / "data" / "romdb.json",
        help="output path (default: qnemu/data/romdb.json)",
    )
    args = parser.parse_args()

    database = build(args.dat_dir)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=args.output.parent, prefix=args.output.name + ".", delete=False
    ) as handle:
        json.dump(database, handle, ensure_ascii=False, separators=(",", ":"))
        handle.write("\n")
        temporary = pathlib.Path(handle.name)
    temporary.replace(args.output)
    print(f"wrote {args.output} ({args.output.stat().st_size:,} bytes)", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

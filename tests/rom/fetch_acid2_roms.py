#!/usr/bin/env python3
"""Download the prebuilt dmg-acid2/cgb-acid2 ROMs used by the PPU tests."""

import argparse
import hashlib
import pathlib
import shutil
import sys
import urllib.request

ROMS = {
    "dmg-acid2.gb": {
        "url": "https://github.com/mattcurrie/dmg-acid2/releases/download/v1.0/dmg-acid2.gb",
        "sha256": "464e14b7d42e7feea0b7ede42be7071dc88913f75b9ffa444299424b63d1dff1",
    },
    "cgb-acid2.gbc": {
        "url": "https://github.com/mattcurrie/cgb-acid2/releases/download/v1.1/cgb-acid2.gbc",
        "sha256": "197fb0bcec544f0400527fc707e0a94f55435974986e6986b424ace5de81720e",
    },
}


def digest(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dest",
        type=pathlib.Path,
        default=pathlib.Path("build/test-roms/acid2"),
    )
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    args.dest.mkdir(parents=True, exist_ok=True)
    for name, info in ROMS.items():
        destination = args.dest / name
        if destination.is_file() and digest(destination) == info["sha256"] and not args.force:
            print(f"{name}: already present")
            continue

        temporary = destination.with_suffix(destination.suffix + ".part")
        print(f"{name}: downloading")
        with urllib.request.urlopen(info["url"], timeout=60) as response, temporary.open("wb") as output:
            shutil.copyfileobj(response, output)

        if digest(temporary) != info["sha256"]:
            temporary.unlink(missing_ok=True)
            print(f"{name}: checksum mismatch", file=sys.stderr)
            return 1
        temporary.replace(destination)
        print(f"{name}: {destination}")

    return 0


if __name__ == "__main__":
    sys.exit(main())

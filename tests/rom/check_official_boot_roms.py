#!/usr/bin/env python3
"""Reject known official Nintendo Game Boy boot ROMs in tracked files.

This is a guard against accidentally committing a dump, not a legal
guarantee. It checks tracked files whose size matches a boot ROM and compares
their SHA-1 against a list of known official dumps. It also rejects
boot-ROM-looking binaries outside the vetted third_party/sameboy/bootroms/
directory, which catches unknown revisions and renamed files.

The hash list comes from the RetroBIOS metadata (accessed 2026-10-31):
https://abdess.github.io/retrobios/systems/nintendo-game-boy/
https://abdess.github.io/retrobios/systems/nintendo/
Hashes are facts; no ROM data is stored here.
"""

from __future__ import annotations

import hashlib
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

# SHA-1 is used only to identify known files, not as a security primitive.
OFFICIAL_SHA1 = {
    "1db57a1e8b6e4096f811587f9eab0c6675fd9755": "DMG",
    "9817bdae9335a3accb584f77c165dc5726555d1f": "MGB",
    "8bd501e31921e9601788316dbd3ce9833a97bcbc": "DMG0",
    "369e6eb5e0c975eaa52a4a3f6ee07b2a3c3c16de": "SGB",
    "f282b3aaf98f8423dab7d77f1aa0192be630f2fb": "SGB2",
    "80e3ee82416c1a74827a5459fde32f04bf51a1f6": "CGB",
    "df5a0d2d49de38fbd31cc2aab8e62c8550e655c0": "CGB0",
    "4ed1d95c9c682a78698f235ce9e9bb3b62bccf9f": "AGB",
    "0daac31acb6cb346fc954368acb02acb3adcc3ab": "AGB0",
    "f5f33729a956131d9c44310f0ae3bb0599e9ec3e": "CGB (alternate)",
    "fa5287e24b0fa533b3b5ef2b28a81245346c1a0f": "AGB (alternate)",
    "1293d68bf9643bc4f36954c1e80e38f39864528d": "CGB (alternate 2)",
}

BOOTROM_SIZES = {256, 2048, 2304}
ALLOWED_DIR = Path("third_party/sameboy/bootroms")
BIN_SUFFIXES = {".bin", ".rom", ".gb", ".gbc"}
BOOT_NAME_RE = re.compile(r"(?i)(boot|bios)")


def tracked_files() -> list[Path]:
    try:
        output = subprocess.check_output(
            ["git", "-C", str(REPO_ROOT), "ls-files", "-z"],
            stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.CalledProcessError):
        # Fallback for source tarballs: walk the tree, skipping build output.
        files = []
        for path in REPO_ROOT.rglob("*"):
            if not path.is_file():
                continue
            rel = path.relative_to(REPO_ROOT)
            if any(part.startswith(".git") or part.startswith("build") for part in rel.parts):
                continue
            files.append(rel)
        return sorted(files)

    return sorted(
        Path(raw.decode("utf-8", "surrogateescape"))
        for raw in output.split(b"\0")
        if raw
    )


def sha1(path: Path) -> str:
    digest = hashlib.sha1()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def is_allowed(path: Path) -> bool:
    try:
        path.relative_to(ALLOWED_DIR)
        return True
    except ValueError:
        return False


def main() -> int:
    failures: list[str] = []
    checked = 0

    for relative in tracked_files():
        path = REPO_ROOT / relative
        if not path.is_file():
            continue
        try:
            size = path.stat().st_size
        except OSError:
            continue
        if size not in BOOTROM_SIZES:
            continue

        checked += 1
        digest = sha1(path)
        known = OFFICIAL_SHA1.get(digest)
        if known:
            failures.append(
                f"{relative}: matches the official {known} boot ROM "
                f"(SHA-1 {digest}); remove it from the repository"
            )
            continue

        if (
            not is_allowed(relative)
            and path.suffix.lower() in BIN_SUFFIXES
            and BOOT_NAME_RE.search(path.name)
        ):
            failures.append(
                f"{relative}: looks like a boot ROM ({size} bytes) outside "
                f"{ALLOWED_DIR}/; move a free reimplementation there or remove it"
            )

    if failures:
        print("Official/unknown boot ROMs found in tracked files:", file=sys.stderr)
        for failure in failures:
            print(f"  - {failure}", file=sys.stderr)
        return 1

    print(
        f"Boot ROM guard: checked {checked} tracked file(s) of boot ROM size; "
        "no official dumps found."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

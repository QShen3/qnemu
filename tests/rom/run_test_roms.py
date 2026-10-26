#!/usr/bin/env python3
"""Run a directory tree of Game Boy test ROMs through gb_test_runner."""

import argparse
import concurrent.futures
import pathlib
import re
import subprocess
import sys


def run_rom(
    runner: pathlib.Path,
    rom: pathlib.Path,
    model: str,
    max_cycles: int,
    max_seconds: int,
):
    command = [
        str(runner),
        str(rom),
        "--model",
        model,
        "--max-cycles",
        str(max_cycles),
        "--max-seconds",
        str(max_seconds),
    ]
    try:
        completed = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=max_seconds + 15,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return rom, 124, "TIMEOUT / UNKNOWN", "wall-clock timeout"

    lines = [line for line in completed.stdout.splitlines() if line.strip()]
    result = lines[-1].strip() if lines else "NO OUTPUT"
    return rom, completed.returncode, result, completed.stdout


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=pathlib.Path, help="directory containing .gb files")
    parser.add_argument(
        "--runner",
        type=pathlib.Path,
        default=pathlib.Path("build/tests/rom/gb_test_runner"),
        help="path to gb_test_runner",
    )
    parser.add_argument("--model", choices=["dmg", "cgb"], default="cgb")
    parser.add_argument("--max-cycles", type=int, default=300_000_000)
    parser.add_argument("--max-seconds", type=int, default=20)
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--include", default="", help="only run paths matching this regex")
    parser.add_argument("--exclude", default="", help="skip paths matching this regex")
    parser.add_argument("--show-failures", action="store_true", help="print failing output")
    args = parser.parse_args()

    if not args.runner.is_file():
        print(f"runner not found: {args.runner}", file=sys.stderr)
        return 2
    if not args.directory.is_dir():
        print(f"directory not found: {args.directory}", file=sys.stderr)
        return 2

    include = re.compile(args.include) if args.include else None
    exclude = re.compile(args.exclude) if args.exclude else None

    roms = []
    for rom in sorted(args.directory.rglob("*.gb")):
        relative = rom.relative_to(args.directory).as_posix()
        if include and not include.search(relative):
            continue
        if exclude and exclude.search(relative):
            continue
        roms.append(rom)

    if not roms:
        print("no ROMs matched", file=sys.stderr)
        return 2

    failures = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
        futures = [
            executor.submit(run_rom, args.runner, rom, args.model, args.max_cycles, args.max_seconds)
            for rom in roms
        ]
        for future in concurrent.futures.as_completed(futures):
            rom, returncode, result, output = future.result()
            relative = rom.relative_to(args.directory).as_posix()
            status = "PASS" if returncode == 0 else "FAIL"
            print(f"{status:4}  {relative}  ({result})")
            if returncode != 0:
                failures.append((relative, returncode, result, output))

    print()
    print(f"{len(roms) - len(failures)}/{len(roms)} passed")

    if failures and args.show_failures:
        for relative, returncode, result, output in failures:
            print()
            print(f"=== {relative} (exit {returncode}, {result}) ===")
            print(output.rstrip())

    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())

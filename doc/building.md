# Building QNEmu

## Requirements

- A C++20 compiler (GCC >= 9, Clang >= 12, or MSVC 2019+)
- CMake >= 3.16 (>= 3.20 for the bundled presets)
- Qt >= 6.3 with Core, Gui, Multimedia, Concurrent, Qml, Quick,
  QuickControls2 and QuickDialogs2
- libarchive >= 3.x (ROM archive support)
- Qt Sensors (system accelerometer input for MBC7)
- Google Test >= 1.10 (only when building the tests)

## Quick start

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The executable is written to `build/qnemu/qnemu`.

## Presets

CMake presets are recommended:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

| Preset     | Description                                   |
| ---------- | --------------------------------------------- |
| `debug`    | Debug build in `build/`                       |
| `release`  | Release build in `build-release/`             |
| `coverage` | Debug build with coverage instrumentation     |
| `analyze`  | Debug build with clang-tidy and cppcheck      |

## Options

| Option                     | Default | Description                                        |
| -------------------------- | ------- | -------------------------------------------------- |
| `BUILD_TESTING`            | `ON`    | Build unit tests (requires Google Test)            |
| `QNEMU_WARNINGS_AS_ERRORS` | `ON`    | Treat compiler warnings as errors                  |
| `QNEMU_ENABLE_CLANG_TIDY`  | `OFF`   | Run clang-tidy while compiling                     |
| `QNEMU_ENABLE_CPPCHECK`    | `OFF`   | Run cppcheck while compiling                       |
| `QNEMU_ENABLE_COVERAGE`    | `OFF`   | Instrument the build and add a `coverage` target   |

## Coverage

```bash
cmake -S . -B build -DQNEMU_ENABLE_COVERAGE=ON
cmake --build build --target coverage
```

This requires `gcovr` for GCC/Clang builds, or OpenCppCoverage for MSVC.

## Installing dependencies

macOS with Homebrew:

```bash
brew install cmake ninja qt googletest libarchive
```

Debian/Ubuntu:

```bash
sudo apt install cmake ninja-build qt6-base-dev qt6-multimedia-dev \
    qt6-declarative-dev libarchive-dev libgtest-dev libgmock-dev
```

Homebrew's libarchive is keg-only; CMake finds it automatically when `brew` is
available. If you use a different prefix, pass
`-DCMAKE_PREFIX_PATH="$(brew --prefix libarchive)"`.

The Qt Quick runtime QML modules (`QtQuick`, `QtQuick.Controls`,
`QtQuick.Dialogs`) are needed to run the GUI; on distros that split them out,
install the `qml6-module-qtquick*` packages for your Qt version.

Qt Sensors is a required build dependency. System accelerometer availability
still depends on the host having a motion sensor; without one, the setting is
locked off and MBC7 games use the keyboard tilt bindings instead.

## Packaging notes

When packaging a macOS `.app` or a Windows build, bundle libarchive (or link it
statically), since Homebrew/vcpkg library paths are machine-specific. libarchive
covers the common ZIP methods (store/deflate/bzip2/lzma/xz/zstd); Deflate64 and
some AES variants are not supported.

When Qt Sensors is linked, also deploy the Sensors library and sensor plugins.
macOS/iOS app bundles need a motion-usage description
(`NSMotionUsageDescription`) for system accelerometer access.

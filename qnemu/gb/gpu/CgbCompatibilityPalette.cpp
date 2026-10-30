/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstddef>
#include <cstdint>

#include "qnemu/gb/gpu/CgbCompatibilityPalette.h"

namespace qnemu
{

namespace
{

struct PaletteCombination
{
    std::array<uint8_t, 3> offset;
};

// CGB boot ROM compatibility palette tables.  The selection algorithm and
// table layout follow the public Pan Docs description, cross-checked against
// SameBoy's MIT-licensed cgb_boot.asm reimplementation
// (https://github.com/LIJI32/SameBoy/blob/master/BootROMs/cgb_boot.asm).
// SameBoy is Copyright (c) 2015-2024 Lior Halphon, MIT licensed.
// Palette values are RGB555, as stored in CGB palette RAM.
constexpr std::array<uint8_t, 94> TitleChecksums = {
    0x00, 0x88, 0x16, 0x36, 0xD1, 0xDB, 0xF2, 0x3C, 0x8C, 0x92, 0x3D, 0x5C, 0x58, 0xC9, 0x3E, 0x70,
    0x1D, 0x59, 0x69, 0x19, 0x35, 0xA8, 0x14, 0xAA, 0x75, 0x95, 0x99, 0x34, 0x6F, 0x15, 0xFF, 0x97,
    0x4B, 0x90, 0x17, 0x10, 0x39, 0xF7, 0xF6, 0xA2, 0x49, 0x4E, 0x43, 0x68, 0xE0, 0x8B, 0xF0, 0xCE,
    0x0C, 0x29, 0xE8, 0xB7, 0x86, 0x9A, 0x52, 0x01, 0x9D, 0x71, 0x9C, 0xBD, 0x5D, 0x6D, 0x67, 0x3F,
    0x6B, 0xB3, 0x46, 0x28, 0xA5, 0xC6, 0xD3, 0x27, 0x61, 0x18, 0x66, 0x6A, 0xBF, 0x0D, 0xF4, 0xB3,
    0x46, 0x28, 0xA5, 0xC6, 0xD3, 0x27, 0x61, 0x18, 0x66, 0x6A, 0xBF, 0x0D, 0xF4, 0xB3,
};
constexpr std::size_t FirstDuplicate = 65;
constexpr std::array<uint8_t, 94> PalettePerChecksum = {
    0, 4, 5, 35, 34, 3, 31, 15, 10, 5, 19, 36, 7, 37, 30, 44,
    21, 32, 31, 20, 5, 33, 13, 14, 5, 29, 5, 18, 9, 3, 2, 26,
    25, 25, 41, 42, 26, 45, 42, 45, 36, 38, 26, 42, 30, 41, 34, 34,
    5, 42, 6, 5, 33, 25, 42, 42, 40, 2, 16, 25, 42, 42, 5, 0,
    39, 36, 22, 25, 6, 32, 12, 36, 11, 39, 18, 39, 24, 31, 50, 17,
    46, 6, 27, 0, 47, 41, 41, 0, 0, 19, 34, 23, 18, 29,
};
constexpr std::array<uint8_t, 29> DuplicateFourthLetters = {
    'B', 'E', 'F', 'A', 'A', 'R', 'B', 'E', 'K', 'E', 'K', ' ', 'R', '-', 'U', 'R',
    'A', 'R', ' ', 'I', 'N', 'A', 'I', 'L', 'I', 'C', 'E', ' ', 'R',
};
constexpr std::array<PaletteCombination, 55> Combinations = {{
    {{32, 32, 232}}, // 0
    {{144, 144, 144}}, // 1
    {{160, 160, 160}}, // 2
    {{192, 192, 192}}, // 3
    {{72, 72, 72}}, // 4
    {{0, 0, 0}}, // 5
    {{216, 216, 216}}, // 6
    {{40, 40, 40}}, // 7
    {{96, 96, 96}}, // 8
    {{208, 208, 208}}, // 9
    {{128, 64, 64}}, // 10
    {{32, 224, 224}}, // 11
    {{32, 16, 16}}, // 12
    {{24, 32, 32}}, // 13
    {{32, 232, 232}}, // 14
    {{224, 32, 224}}, // 15
    {{16, 136, 16}}, // 16
    {{128, 128, 64}}, // 17
    {{32, 32, 56}}, // 18
    {{32, 32, 144}}, // 19
    {{32, 32, 160}}, // 20
    {{152, 152, 72}}, // 21
    {{30, 30, 88}}, // 22
    {{136, 136, 16}}, // 23
    {{32, 32, 16}}, // 24
    {{32, 32, 24}}, // 25
    {{224, 224, 0}}, // 26
    {{24, 24, 0}}, // 27
    {{0, 0, 8}}, // 28
    {{144, 176, 144}}, // 29
    {{160, 176, 160}}, // 30
    {{192, 176, 192}}, // 31
    {{128, 176, 64}}, // 32
    {{136, 32, 104}}, // 33
    {{222, 0, 112}}, // 34
    {{222, 32, 120}}, // 35
    {{152, 182, 72}}, // 36
    {{128, 224, 80}}, // 37
    {{32, 184, 224}}, // 38
    {{136, 176, 16}}, // 39
    {{32, 0, 16}}, // 40
    {{32, 224, 24}}, // 41
    {{224, 24, 0}}, // 42
    {{24, 224, 32}}, // 43
    {{168, 224, 32}}, // 44
    {{24, 224, 0}}, // 45
    {{200, 24, 224}}, // 46
    {{0, 224, 64}}, // 47
    {{32, 24, 224}}, // 48
    {{224, 24, 48}}, // 49
    {{32, 224, 232}}, // 50
    {{240, 240, 240}}, // 51
    {{248, 248, 248}}, // 52
    {{224, 32, 8}}, // 53
    {{0, 0, 16}}, // 54
}};
constexpr std::array<uint16_t, 128> PaletteWords = {
    0x7FFF, 0x32BF, 0x00D0, 0x0000, 0x639F, 0x4279, 0x15B0, 0x04CB,
    0x7FFF, 0x6E31, 0x454A, 0x0000, 0x7FFF, 0x1BEF, 0x0200, 0x0000,
    0x7FFF, 0x421F, 0x1CF2, 0x0000, 0x7FFF, 0x5294, 0x294A, 0x0000,
    0x7FFF, 0x03FF, 0x012F, 0x0000, 0x7FFF, 0x03EF, 0x01D6, 0x0000,
    0x7FFF, 0x42B5, 0x3DC8, 0x0000, 0x7E74, 0x03FF, 0x0180, 0x0000,
    0x67FF, 0x77AC, 0x1A13, 0x2D6B, 0x7ED6, 0x4BFF, 0x2175, 0x0000,
    0x53FF, 0x4A5F, 0x7E52, 0x0000, 0x4FFF, 0x7ED2, 0x3A4C, 0x1CE0,
    0x03ED, 0x7FFF, 0x255F, 0x0000, 0x036A, 0x021F, 0x03FF, 0x7FFF,
    0x7FFF, 0x01DF, 0x0112, 0x0000, 0x231F, 0x035F, 0x00F2, 0x0009,
    0x7FFF, 0x03EA, 0x011F, 0x0000, 0x299F, 0x001A, 0x000C, 0x0000,
    0x7FFF, 0x027F, 0x001F, 0x0000, 0x7FFF, 0x03E0, 0x0206, 0x0120,
    0x7FFF, 0x7EEB, 0x001F, 0x7C00, 0x7FFF, 0x3FFF, 0x7E00, 0x001F,
    0x7FFF, 0x03FF, 0x001F, 0x0000, 0x03FF, 0x001F, 0x000C, 0x0000,
    0x7FFF, 0x033F, 0x0193, 0x0000, 0x0000, 0x4200, 0x037F, 0x7FFF,
    0x7FFF, 0x7E8C, 0x7C00, 0x0000, 0x7FFF, 0x1BEF, 0x6180, 0x0000,
    0x7FFF, 0x7FEA, 0x7D5F, 0x0000, 0x4778, 0x3290, 0x1D87, 0x0861,
};

static_assert(TitleChecksums.size() == PalettePerChecksum.size());
static_assert(TitleChecksums.size() - FirstDuplicate == DuplicateFourthLetters.size());
static_assert(Combinations.size() == 55);
static_assert(PaletteWords.size() == 32 * 4);

std::array<uint16_t, 4> paletteAt(uint8_t byteOffset)
{
    const std::size_t index = static_cast<std::size_t>(byteOffset) / 2;
    std::array<uint16_t, 4> colors {};
    for (std::size_t i = 0; i < colors.size(); ++i) {
        colors.at(i) = PaletteWords.at(index + i);
    }
    return colors;
}

DmgCompatibilityPalette paletteForCombination(std::uint8_t combination)
{
    if (combination >= Combinations.size()) {
        combination = 0;
    }
    const auto& offsets = Combinations.at(combination).offset;
    return {
        paletteAt(offsets.at(0)),
        paletteAt(offsets.at(1)),
        paletteAt(offsets.at(2)),
    };
}

}  // namespace

DmgCompatibilityPalette cgbCompatibilityPalette(const CgbCompatibilityHeader& header)
{
    const bool nintendo = header.oldLicensee == 0x33
        ? (header.newLicensee.at(0) == '0' && header.newLicensee.at(1) == '1')
        : header.oldLicensee == 0x01;
    if (!nintendo) {
        return paletteForCombination(0);
    }

    std::uint8_t checksum = 0;
    for (const std::uint8_t byte : header.title) {
        checksum = static_cast<std::uint8_t>(checksum + byte);
    }

    for (std::size_t i = 0; i < TitleChecksums.size(); ++i) {
        if (TitleChecksums.at(i) != checksum) {
            continue;
        }
        if (i < FirstDuplicate
            || DuplicateFourthLetters.at(i - FirstDuplicate) == header.title.at(3)) {
            return paletteForCombination(
                static_cast<std::uint8_t>(PalettePerChecksum.at(i) & 0x7F));
        }
    }
    return paletteForCombination(0);
}

}  // namespace qnemu

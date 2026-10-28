/*
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace qnemu
{

inline constexpr uint32_t CpuFrequency = 4194304;

inline constexpr uint16_t MemoryRomBank00Start = 0x0000;
inline constexpr uint16_t MemoryRomBank00End = 0x3FFF;
inline constexpr uint16_t MemoryRomBank01Start = 0x4000;
inline constexpr uint16_t MemoryRomBank01End = 0x7FFF;

inline constexpr uint16_t VideoRamStart = 0x8000;
inline constexpr uint16_t VideoRamEnd = 0x9FFF;

inline constexpr uint16_t ExternalRamStart = 0xA000;
inline constexpr uint16_t ExternalRamEnd = 0xBFFF;

inline constexpr uint16_t WorkRamBank00Start = 0xC000;
inline constexpr uint16_t WorkRamBank00End = 0xCFFF;
inline constexpr uint16_t WorkRamBank01Start = 0xD000;
inline constexpr uint16_t WorkRamBank01End = 0xDFFF;

inline constexpr uint16_t EchoRamStart = 0xE000;
inline constexpr uint16_t EchoRamEnd = 0xFDFF;

inline constexpr uint16_t OamStart = 0xFE00;
inline constexpr uint16_t OamEnd = 0xFE9F;

inline constexpr uint16_t HighRamStart = 0xFF80;
inline constexpr uint16_t HighRamEnd = 0xFFFE;

inline constexpr uint32_t MemorySpaceSize = 0x10000;

inline constexpr std::size_t RomBankSize = 0x4000;
inline constexpr std::size_t RamBankSize = 0x2000;
inline constexpr std::size_t VideoRamBankSize = 0x2000;
inline constexpr std::size_t OamSize = 0xA0;

}  // namespace qnemu

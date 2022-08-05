/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace qnemu
{

static constexpr uint16_t MemoryRomBank00Start = 0;
static constexpr uint16_t MemoryRomBank00End = 0x3FFF;

static constexpr uint16_t MemoryRomBank01Start = 0x4000;
static constexpr uint16_t MemoryRomBank01End = 0x7FFF;

static constexpr uint16_t VideoRamStart = 0x8000;
static constexpr uint16_t VideoRamEnd = 0x9FFF;

static constexpr uint16_t ExternalRamStart = 0xA000;
static constexpr uint16_t ExternalRamEnd = 0xBFFF;

static constexpr uint16_t WorkRamBank00Start = 0xC000;
static constexpr uint16_t WorkRamBank00End = 0xCFFF;

static constexpr uint16_t WorkRamBank01Start = 0xD000;
static constexpr uint16_t WorkRamBank01End = 0xDFFF;

static constexpr uint16_t EchoRamStart = 0xE000;
static constexpr uint16_t EchoRamEnd = 0xFDFF;

static constexpr uint16_t SpriteAttributeTableStart = 0xFE00;
static constexpr uint16_t SpriteAttributeTableEnd = 0xFE9F;

static constexpr uint16_t HighRamStart = 0xFE80;
static constexpr uint16_t HighRamEnd = 0xFFFE;


static constexpr size_t RomBankSize = 0x4000;
static constexpr size_t RamBankSize = 0x2000;

static constexpr size_t VideoRamBankSize = 0x2000;
static constexpr size_t SpriteAttributeTableSize = 0xA0;

}  // namespace qnemu

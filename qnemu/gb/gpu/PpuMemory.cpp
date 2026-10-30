/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/PpuMemory.h"

namespace qnemu
{

PpuMemory::PpuMemory()
{
    reset();
}

void PpuMemory::reset()
{
    for (auto& bank : videoRam_) {
        bank.fill(0x00);
    }
    oam_.fill(0x00);
    backgroundPalette_.fill(0x00);
    objectPalette_.fill(0x00);
    vramBank_ = 0x00;
    backgroundPaletteSpec_ = 0x00;
    objectPaletteSpec_ = 0x00;
    dmgCompatibility_ = false;
}

uint8_t PpuMemory::readVram(uint16_t address) const
{
    return videoRam_.at(vramBank_).at(address - VideoRamStart);
}

void PpuMemory::writeVram(uint16_t address, uint8_t value)
{
    videoRam_.at(vramBank_).at(address - VideoRamStart) = value;
}

uint8_t PpuMemory::readVramByte(uint8_t bank, uint16_t offset) const
{
    return videoRam_.at(bank & 0x01).at(offset);
}

void PpuMemory::writeVramByte(uint8_t bank, uint16_t offset, uint8_t value)
{
    videoRam_.at(bank & 0x01).at(offset) = value;
}

uint8_t PpuMemory::readOam(uint16_t address) const
{
    return oam_.at(address - OamStart);
}

void PpuMemory::writeOam(uint16_t address, uint8_t value)
{
    oam_.at(address - OamStart) = value;
}

uint8_t PpuMemory::readOamByte(uint8_t offset) const
{
    return oam_.at(offset);
}

void PpuMemory::writeOamByte(uint8_t offset, uint8_t value)
{
    oam_.at(offset) = value;
}

uint8_t PpuMemory::readPaletteSpec(bool object) const
{
    return object ? objectPaletteSpec_ : backgroundPaletteSpec_;
}

void PpuMemory::writePaletteSpec(bool object, uint8_t value)
{
    if (object) {
        objectPaletteSpec_ = value;
    } else {
        backgroundPaletteSpec_ = value;
    }
}

uint8_t PpuMemory::readPaletteData(bool object) const
{
    const uint8_t index = static_cast<uint8_t>((object ? objectPaletteSpec_ : backgroundPaletteSpec_) & 0x3F);
    return object ? objectPalette_.at(index) : backgroundPalette_.at(index);
}

void PpuMemory::writePaletteData(bool object, uint8_t value)
{
    uint8_t& spec = object ? objectPaletteSpec_ : backgroundPaletteSpec_;
    const uint8_t index = static_cast<uint8_t>(spec & 0x3F);
    if (object) {
        objectPalette_.at(index) = value;
    } else {
        backgroundPalette_.at(index) = value;
    }
    if ((spec & 0x80) != 0) {
        spec = static_cast<uint8_t>((spec & 0x80) | ((index + 1) & 0x3F));
    }
}

uint32_t PpuMemory::dmgColor(uint8_t palette, uint8_t colorIndex) const
{
    const uint8_t shade = static_cast<uint8_t>((palette >> (colorIndex * 2)) & 0x03);
    switch (shade) {
        case 0: return rgb(255, 255, 255);
        case 1: return rgb(170, 170, 170);
        case 2: return rgb(85, 85, 85);
        default: return rgb(0, 0, 0);
    }
}


void PpuMemory::setDmgCompatibilityPalette(const DmgCompatibilityPalette& palette)
{
    writePaletteBlock(backgroundPalette_, 0, palette.background);
    writePaletteBlock(objectPalette_, 0, palette.object0);
    writePaletteBlock(objectPalette_, 1, palette.object1);
    dmgCompatibility_ = true;
}

void PpuMemory::writePaletteBlock(std::array<uint8_t, 0x40>& destination, uint8_t palette,
    const std::array<uint16_t, 4>& colors)
{
    const std::size_t base = static_cast<std::size_t>(palette) * 8;
    for (std::size_t i = 0; i < colors.size(); ++i) {
        const uint16_t color = colors.at(i);
        destination.at(base + i * 2) = static_cast<uint8_t>(color & 0xFF);
        destination.at(base + i * 2 + 1) = static_cast<uint8_t>(color >> 8);
    }
}

uint32_t PpuMemory::dmgCompatibilityColor(bool object, uint8_t palette, uint8_t dmgPalette,
    uint8_t colorIndex) const
{
    const uint8_t shade = static_cast<uint8_t>((dmgPalette >> (colorIndex * 2)) & 0x03);
    return cgbColor(object, palette, shade);
}

uint32_t PpuMemory::cgbColor(bool object, uint8_t palette, uint8_t colorIndex) const
{
    const uint8_t index = static_cast<uint8_t>(((palette & 0x07) * 8) + ((colorIndex & 0x03) * 2));
    const auto& data = object ? objectPalette_ : backgroundPalette_;
    const uint16_t color = static_cast<uint16_t>(data.at(index))
        | (static_cast<uint16_t>(data.at(index + 1)) << 8);
    const uint8_t red = static_cast<uint8_t>(color & 0x1F);
    const uint8_t green = static_cast<uint8_t>((color >> 5) & 0x1F);
    const uint8_t blue = static_cast<uint8_t>((color >> 10) & 0x1F);
    return rgb(static_cast<uint8_t>((red << 3) | (red >> 2)),
        static_cast<uint8_t>((green << 3) | (green >> 2)),
        static_cast<uint8_t>((blue << 3) | (blue >> 2)));
}

}  // namespace qnemu

/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <cstring>

#include <QtGui/QColor>

#include "qnemu/gb/gpu/GbcPalette.h"

namespace qnemu
{

GbcPalette::GbcPalette()
{
    GbcPalette::reset();
}

uint8_t GbcPalette::read(uint16_t address) const
{
    if (address == 0xFF68) {
        return registers.gbcBackgroundPaletteSpecification;
    }
    if (address == 0xFF69) {
        return registers.gbcBckgroundPaletteData;
    }
    if (address == 0xFF6A) {
        return registers.gbcSpritePaletteSpecification;
    }
    if (address == 0xFF6B) {
        return registers.gbcSpritePaletteData;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbcPalette::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF68) {
        registers.gbcBackgroundPaletteSpecification = value;
    }
    if (address == 0xFF69) {
        registers.gbcBckgroundPaletteData = value;
        backgroundOrWindowData.at(registers.gbcBackgroundPaletteIndex) = value;
        if (registers.gbcBackgroundAutoIncrement) {
            registers.gbcBackgroundPaletteIndex++;
        }
    }
    if (address == 0xFF6A) {
        registers.gbcSpritePaletteSpecification = value;
    }
    if (address == 0xFF6B) {
        registers.gbcSpritePaletteData = value;
        spriteData.at(registers.gbcSritePaletteIndex) = value;
        if (registers.gbcSpriteAutoIncrement) {
            registers.gbcSritePaletteIndex++;
        }
    }
}

void GbcPalette::step()
{
}

void GbcPalette::reset()
{
    std::memset(&registers, 0, sizeof(registers));
}

QRgb GbcPalette::getBackgroundColor(uint16_t colorIndex) const
{
    const uint16_t colorNumber = (static_cast<uint16_t>(backgroundOrWindowData.at(colorIndex + 1)) << 8) | static_cast<uint16_t>(backgroundOrWindowData.at(colorIndex));
    const GbcColor color = { .color = colorNumber };
    return qRgb((color.red << 3 | color.red >> 2), (color.green << 3 | color.green >> 2), (color.blue << 3 | color.blue >> 2));
}

QRgb GbcPalette::getSpriteColor(uint16_t colorIndex) const
{
    const uint16_t colorNumber = (static_cast<uint16_t>(spriteData.at(colorIndex + 1)) << 8) | static_cast<uint16_t>(spriteData.at(colorIndex));
    const GbcColor color = { .color = colorNumber };
    return qRgb((color.red << 3 | color.red >> 2), (color.green << 3 | color.green >> 2), (color.blue << 3 | color.blue >> 2));
}

}  // namespace qnemu

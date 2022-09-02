/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/gpu/GbPalette.h"

namespace qnemu
{

GbPalette::GbPalette()
{
    GbPalette::reset();
}

bool GbPalette::accepts(uint16_t address) const
{
    if (address >= 0xFF47 && address <= 0xFF49) {
        return true;
    }
    return false;
}

uint8_t GbPalette::read(uint16_t address) const
{
    if (address == 0xFF47) {
        return registers.backgroundPaletteData;
    }
    if (address == 0xFF48) {
        return registers.spritePalette0Data;
    }
    if (address == 0xFF49) {
        return registers.spritePalette1Data;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbPalette::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF47) {
        registers.backgroundPaletteData = value;
    }
    if (address == 0xFF48) {
        registers.spritePalette0Data = value;
    }
    if (address == 0xFF49) {
        registers.spritePalette1Data = value;
    }
}

void GbPalette::step()
{
}

void GbPalette::reset()
{
    registers.backgroundPaletteData = 0xFC;
}

QRgb GbPalette::getBackgroundColor(uint16_t colorIndex) const
{
    uint8_t shade = 0;
    shade = static_cast<uint8_t>(registers.backgroundPaletteData << (6 - colorIndex * 2)) >> 6;

    return shadeToQRgb(shade);
}

QRgb GbPalette::getSpriteColor(uint16_t colorIndex, uint8_t paletteIndex) const
{
    assert((paletteIndex == 0 || paletteIndex == 1) && "Wrong palette index!");
    uint8_t shade = 0;
    uint8_t paletteData = paletteIndex == 0 ? registers.spritePalette0Data : registers.spritePalette1Data;
    shade = static_cast<uint8_t>(paletteData << (6 - colorIndex * 2)) >> 6;

    return shadeToQRgb(shade);
}

QRgb GbPalette::shadeToQRgb(uint8_t shade) const
{
    if (shade == 0) {
        return qRgb(255, 255, 255);
    }
    if (shade == 1) {
        return qRgb(170, 170, 170);
    }
    if (shade == 2) {
        return qRgb(85, 85, 85);
    }
    if (shade == 3) {
        return qRgb(0, 0, 0);
    }
    return qRgb(0xFF, 0xFF, 0xFF);
}

}  // namespace qnemu

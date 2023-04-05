/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include <QtGui/QColor>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbcPalette : public GbDeviceInterface
{
public:
    GbcPalette();
    ~GbcPalette() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    QRgb getBackgroundColor(uint16_t colorIndex) const;
    QRgb getSpriteColor(uint16_t colorIndex) const;

private:
    union GbcColor {
        struct {
            uint red : 5;
            uint green : 5;
            uint blue : 5;
            uint : 1;
        };
        uint16_t color;
    };
    struct {
        union {
            struct {
                uint8_t gbcBackgroundPaletteIndex : 6;
                uint8_t : 1;
                uint8_t gbcBackgroundAutoIncrement : 1;
            };
            uint8_t gbcBackgroundPaletteSpecification;
        };  // FF68
        uint8_t gbcBckgroundPaletteData;  // FF69
        union {
            struct {
                uint8_t gbcSritePaletteIndex : 6;
                uint8_t : 1;
                uint8_t gbcSpriteAutoIncrement : 1;
            };
            uint8_t gbcSpritePaletteSpecification;
        };  // FF6A
        uint8_t gbcSpritePaletteData;  // FF6B
    } registers;
    std::array<uint8_t, 0x40> backgroundOrWindowData;
    std::array<uint8_t, 0x40> spriteData;
};

}  // namespace qnemu

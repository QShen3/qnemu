/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include <QtGui/QColor>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbPalette : public GbDeviceInterface
{
public:
    GbPalette();
    ~GbPalette() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    QRgb getBackgroundColor(uint16_t colorIndex) const;
    QRgb getSpriteColor(uint16_t colorIndex, uint8_t paletteIndex) const;

private:
    static QRgb shadeToQRgb(uint8_t shade);
    struct {
        union {
            struct {
                uint8_t shadeForBackgroundColorIndex0 : 2;
                uint8_t shadeForBackgroundColorIndex1 : 2;
                uint8_t shadeForBackgroundColorIndex2 : 2;
                uint8_t shadeForBackgroundColorIndex3 : 2;
            };
            uint8_t backgroundPaletteData;
        };  // FF47
        union {
            struct {
                uint8_t shadeForSprite0ColorIndex0 : 2;
                uint8_t shadeForSprite0ColorIndex1 : 2;
                uint8_t shadeForSprite0ColorIndex2 : 2;
                uint8_t shadeForSprite0ColorIndex3 : 2;
            };
            uint8_t spritePalette0Data;
        };  // FF48
        union {
            struct {
                uint8_t shadeForSprite1ColorIndex0 : 2;
                uint8_t shadeForSprite1ColorIndex1 : 2;
                uint8_t shadeForSprite1ColorIndex2 : 2;
                uint8_t shadeForSprite1ColorIndex3 : 2;
            };
            uint8_t spritePalette1Data;
        };  // FF49
    } registers;
};

}  // namespace qnemu

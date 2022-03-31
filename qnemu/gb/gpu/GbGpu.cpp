/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <QtGui/QColor>

#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/gpu/Mode.h"

namespace qnemu
{

GbGpu::GbGpu(const GbCartridgeInterface& cartridge, std::shared_ptr<GbInterruptHandler> interruptHandler) :
    cartridge(cartridge),
    interruptHandler(interruptHandler),
    output(160, 144, QImage::Format_RGB32),
    modes({
        Mode
        { "Mode0", 204, [this](){mode0();} },
        { "Mode1", 456, [this](){mode1();} },
        { "Mode2", 80,  [this](){mode2();} },
        { "Mode3", 172, [this](){mode3();} },
    })
{
    GbGpu::reset();
}

bool GbGpu::accepts(uint16_t address) const
{
    if (address >= 0x8000 && address < 0xA000) {
        return true;
    }
    if (address >= 0xFE00 && address < 0xFEA0) {
        return true;
    }
    if (address >= 0xFF40 && address <= 0xFF4B) {
        return true;
    }
    if (address == 0xFF4F) {
        return true;
    }
    if (address >= 0xFF51 && address <= 0xFF55) {
        return true;
    }
    if (address >= 0xFF68 && address <= 0xFF6B) {
        return true;
    }
    return false;
}

uint8_t GbGpu::read(uint16_t address) const
{
    if (address >= 0x8000 && address < 0xA000) {
        return videoRamBanks.at(registers.videoRamBank).at(address - 0x8000);
    }
    if (address >= 0xFE00 && address < 0xFEA0) {
        return spriteAttributeTable.at(address - 0xFE00);
    }
    if (address == 0xFF40) {
        return registers.lcdControl;
    }
    if (address == 0xFF41) {
        return registers.lcdStatus;
    }
    if (address == 0xFF42) {
        return registers.scrollY;
    }
    if (address == 0xFF43) {
        return registers.scrollX;
    }
    if (address == 0xFF44) {
        return registers.lcdYCoordinate;
    }
    if (address == 0xFF45) {
        return registers.lcdYCoordinateCompare;
    }
    if (address == 0xFF46) {
        return registers.dMATransferAndStartAddress;
    }
    if (address == 0xFF47) {
        return registers.backgroundPaletteData;
    }
    if (address == 0xFF48) {
        return registers.spritePalette0Data;
    }
    if (address == 0xFF49) {
        return registers.spritePalette1Data;
    }
    if (address == 0xFF4A) {
        return registers.windowYPosition;
    }
    if (address == 0xFF4B) {
        return registers.windowXPosition;
    }
    if (address == 0xFF4F) {
        return registers.videoRamBank;
    }
    if (address == 0xFF51) {
        return registers.newDMASourceHigh;
    }
    if (address == 0xFF52) {
        return registers.newDMASourceLow;
    }
    if (address == 0xFF53) {
        return registers.newDMADestinationHigh;
    }
    if (address == 0xFF54) {
        return registers.newDMADestinationLow;
    }
    if (address == 0xFF55) {
        return registers.newDMALength;
    }
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

void GbGpu::write(uint16_t address, const uint8_t& value)
{
    if (address >= 0x8000 && address < 0xA000) {
        videoRamBanks.at(registers.videoRamBank).at(address - 0x8000) = value;
    }
    if (address >= 0xFE00 && address < 0xFEA0) {
        spriteAttributeTable.at(address - 0xFE00) = value;
    }
    if (address == 0xFF40) {
        registers.lcdControl = value;
    }
    if (address == 0xFF41) {
        registers.lcdStatus = value;
    }
    if (address == 0xFF42) {
        registers.scrollY = value;
    }
    if (address == 0xFF43) {
        registers.scrollX = value;
    }
    if (address == 0xFF44) {
        registers.lcdYCoordinate = value;
    }
    if (address == 0xFF45) {
        registers.lcdYCoordinateCompare = value;
    }
    if (address == 0xFF46) {
        registers.dMATransferAndStartAddress = value;
    }
    if (address == 0xFF47) {
        registers.backgroundPaletteData = value;
    }
    if (address == 0xFF48) {
        registers.spritePalette0Data = value;
    }
    if (address == 0xFF49) {
        registers.spritePalette1Data = value;
    }
    if (address == 0xFF4A) {
        registers.windowYPosition = value;
    }
    if (address == 0xFF4B) {
        registers.windowXPosition = value;
    }
    if (address == 0xFF4F) {
        registers.videoRamBank = value;
    }
    if (address == 0xFF51) {
        registers.newDMASourceHigh = value;
    }
    if (address == 0xFF52) {
        registers.newDMASourceLow = value;
    }
    if (address == 0xFF53) {
        registers.newDMADestinationHigh = value;
    }
    if (address == 0xFF54) {
        registers.newDMADestinationLow = value;
    }
    if (address == 0xFF55) {
        registers.newDMALength = value;
    }
    if (address == 0xFF68) {
        registers.gbcBackgroundPaletteSpecification = value;
    }
    if (address == 0xFF69) {
        registers.gbcBckgroundPaletteData = value;
    }
    if (address == 0xFF6A) {
        registers.gbcSpritePaletteSpecification = value;
    }
    if (address == 0xFF6B) {
        registers.gbcSpritePaletteData = value;
    }
}

void GbGpu::step()
{
    if (registers.lcdEnable == 0) {
        ticks = 0;
        return;
    }
    if (ticks == 0) {
        auto mode = modes.at(registers.modeFlag);
        ticks = mode.ticks;
        mode.execute();
    } else {
        ticks--;
    }
}

void GbGpu::reset()
{
    std::memset(&registers, 0, sizeof(registers));
    registers.lcdControl = 0x91;
    registers.lcdStatus = 0x85;
    registers.backgroundPaletteData = 0xFC;
    registers.videoRamBank = 0xFF;
    registers.newDMASourceHigh = 0xFF;
    registers.newDMASourceLow = 0xFF;
    registers.newDMADestinationHigh = 0xFF;
    registers.newDMADestinationLow = 0xFF;
    registers.newDMALength = 0xFF;

    ticks = 0;
}

void GbGpu::checklcdYCoordinate()
{
    if (registers.lcdYCoordinate == registers.lcdYCoordinateCompare) {
        registers.coincidenceFlag = 1;
        if (registers.lyCoincidenceInterrupt) {
            interruptHandler->registers.lcdRequest = 1;
        }
    }
    else {
        registers.coincidenceFlag = 0;
    }
}

QRgb GbGpu::getColor(uint16_t) const
{
    return qRgb(0xFF, 0xFF, 0xFF);
}

uint16_t GbGpu::getColorIndexOfBackgroundOrWindow(uint8_t x, uint8_t y, size_t tileMapOffset)
{
    uint8_t tileX = x / 8, tileY = y / 8;
    GbcTileAttribute tileAttribute { .attribute = 0};
    uint8_t tileIndex = videoRamBanks[0].at(tileMapOffset + (tileY * 32 + tileX));
    if (cartridge.isGbcCartridge()) {
        tileAttribute.attribute = videoRamBanks[1].at(tileMapOffset + (tileY * 32 + tileX));
    }
    if (tileAttribute.backgroundToOAMPriority == 1) {
        backgroundToOAMPriorityMap[x][y] = true;
    }

    bool isBank1 = cartridge.isGbcCartridge() && (tileAttribute.tileVideoRamBank == 1);
    uint8_t pixelXinTile = x % 8, pixelYinTile = y % 8;
    uint8_t bitPositionInByte = tileAttribute.horizontalFlip ? 1 << pixelXinTile : 1 << (7 - pixelXinTile);
    size_t byteIndex = tileIndex * 16 + (tileAttribute.verticalFlip ? (7 - pixelYinTile) : pixelYinTile) * 2;
    if (tileIndex < 128) {
        byteIndex += registers.backgroundAndWindowTileDataArea == 1 ? 0 : 0x1000;
    }
    const auto& videoRamBank = isBank1 ? videoRamBanks[1] : videoRamBanks[0];
    uint8_t colorIndex = ((videoRamBank.at(byteIndex) & bitPositionInByte) ? 1 : 0) + ((videoRamBank.at(byteIndex + 1) & bitPositionInByte) ? 2 : 0);
    colorIndexMap[x][y] = colorIndex;
    return colorIndex;
}

void GbGpu::renderLine()
{
    if (registers.lcdEnable == 0) {
        return;
    }
    for (auto &data : backgroundToOAMPriorityMap) {
        std::fill_n(data.begin(), data.size(), false);
    }
    bool isWindowVisible = false;
    QRgb *line = reinterpret_cast<QRgb*>(output.scanLine(registers.lcdYCoordinate));
    std::fill_n(line, output.bytesPerLine(), 0xFFFFFFFF);
    for (uint8_t i = 0; i < 160; i++) {
        if (cartridge.isGbcCartridge() || registers.backgroundAndWindowPriority == 1) {
            uint8_t x = (i + registers.scrollX) % 256;
            uint8_t y = (registers.lcdYCoordinate + registers.scrollY) % 256;
            size_t tileMapOffset = registers.backgroundTileMapArea == 0 ? 0x1800 : 0x1c00;
            uint8_t colorIndex = getColorIndexOfBackgroundOrWindow(x, y, tileMapOffset);
            line[i] = getColor(colorIndex);
        }
        if ((cartridge.isGbcCartridge() || registers.backgroundAndWindowPriority == 1) &&
                (registers.windowEnable == 1 && (registers.windowXPosition - 7) <= i && registers.windowYPosition <= registers.lcdYCoordinate)) {
            uint8_t x = i - registers.windowXPosition + 7;
            uint8_t y = windowLineCounter;
            size_t tileMapOffset = registers.windowTileMapArea == 0 ? 0x1800 : 0x1c00;
            uint8_t colorIndex = getColorIndexOfBackgroundOrWindow(x, y, tileMapOffset);
            line[i] = getColor(colorIndex);
            isWindowVisible = true;
        }
    }
    if (isWindowVisible) {
        windowLineCounter++;
    }
    if (registers.spriteEnable == 0) {
        return;
    }
}

void GbGpu::scanSprites()
{
    spriteStack = {};
    for (uint8_t i = 0; i < 160; i += 4) {
        int16_t y = spriteAttributeTable.at(i) - 16;
        if (registers.spriteSize == 1) {
            if (int16_t(registers.lcdYCoordinate) >= y && int16_t(registers.lcdYCoordinate) < (y + 16)) {
                spriteStack.push(i);
            }
        } else {
            if (int16_t(registers.lcdYCoordinate) >= y && int16_t(registers.lcdYCoordinate) < (y + 8)) {
                spriteStack.push(i);
            }
        }
        if (spriteStack.size() >= 10) {
            break;
        }
    }
}

}  // namespace qnemu
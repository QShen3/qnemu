/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <tuple>
#include <utility>

#include <QtGui/QColor>
#include <QtGui/QImage>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/Mode.h"
#include "qnemu/gb/gpu/SpriteAttributeTable.h"

namespace qnemu
{

GbGpu::GbGpu(const GbCartridgeInterface& cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandler> interruptHandler,
        std::unique_ptr<GbcPalette> gbcPalette,
        std::unique_ptr<SpriteAttributeTable> spriteAttributeTable) :
    cartridge(cartridge),
    display(display),
    interruptHandler(interruptHandler),
    gbcPalette(std::move(gbcPalette)),
    spriteAttributeTable(std::move(spriteAttributeTable)),
    modes({
        Mode
        { "Mode0", 204, [this](){mode0();} },
        { "Mode1", 456, [this](){mode1();} },
        { "Mode2", 80,  [this](){mode2();} },
        { "Mode3", 172, [this](){mode3();} },
    })
{
    GbGpu::reset();
    subDevices.push_back(*(this->gbcPalette));
    subDevices.push_back(*(this->spriteAttributeTable));
}

GbGpu::~GbGpu()
{
}

bool GbGpu::accepts(uint16_t address) const
{
    for (const auto& subDevice : subDevices) {
        if (subDevice.get().accepts(address)) {
            return true;
        }
    }
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        return true;
    }
    if (address >= 0xFF40 && address <= 0xFF45) {
        return true;
    }
    if (address >= 0xFF47 && address <= 0xFF4B) {
        return true;
    }
    if (address == 0xFF4F) {
        return true;
    }
    if (address >= 0xFF51 && address <= 0xFF55) {
        return true;
    }
    return false;
}

uint8_t GbGpu::read(uint16_t address) const
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        if (registers.modeFlag == 3 && registers.lcdEnable == 1) {
            return 0xFF;
        }
        return videoRamBanks.at(cartridge.isGbcCartridge() ? registers.videoRamBank : 0).at(address - VideoRamStart);
    }
    if (address >= SpriteAttributeTableStart && address <= SpriteAttributeTableEnd) {
        if ((registers.modeFlag == 2 || registers.modeFlag == 3) && registers.lcdEnable == 1) {
            return 0xFF;
        }
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
    if (address == 0xFF69) {
        if (registers.modeFlag == 3 && registers.lcdEnable == 1) {
            return 0xFF;
        }
    }
    if (address == 0xFF6B) {
        if (registers.modeFlag == 3 && registers.lcdEnable == 1) {
            return 0xFF;
        }
    }
    for (const auto& subDevice : subDevices) {
        if (subDevice.get().accepts(address)) {
            return subDevice.get().read(address);
        }
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbGpu::write(uint16_t address, const uint8_t& value)
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        if (registers.modeFlag == 3 && registers.lcdEnable == 1) {
            return;
        }
        videoRamBanks.at(cartridge.isGbcCartridge() ? registers.videoRamBank : 0).at(address - VideoRamStart) = value;
    }
    if (address >= SpriteAttributeTableStart && address <= SpriteAttributeTableEnd) {
        if ((registers.modeFlag == 2 || registers.modeFlag == 3) && registers.lcdEnable == 1) {
            return;
        }
    }
    if (address == 0xFF40) {
        registers.lcdControl = value;
    }
    if (address == 0xFF41) {
        uint8_t valueToWrite = value & 0b11111000;
        valueToWrite |= (registers.lcdStatus & 0b00000111);
        registers.lcdStatus = valueToWrite;
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
    if (address == 0xFF69) {
        if (registers.modeFlag == 3 && registers.lcdEnable == 1) {
            return;
        }
    }
    if (address == 0xFF6B) {
        if (registers.modeFlag == 3 && registers.lcdEnable == 1) {
            return;
        }
    }
    for (auto& subDevice : subDevices) {
        if (subDevice.get().accepts(address)) {
            return subDevice.get().write(address, value);
        }
    }
}

void GbGpu::step()
{
    for (auto& subDevice : subDevices) {
        subDevice.get().step();
    }
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

    for (auto& subDevice : subDevices) {
        subDevice.get().reset();
    }
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

QRgb GbGpu::getGbColor(uint16_t colorIndex, uint8_t paletteData) const
{
    uint8_t shade = 0;
    shade = static_cast<uint8_t>(paletteData << (6 - colorIndex * 2)) >> 6;

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

std::tuple<uint16_t, bool> GbGpu::getColorIndexAndPriorityOfBackgroundOrWindow(uint8_t x, uint8_t y, size_t tileMapOffset) const
{
    uint8_t tileX = x / 8, tileY = y / 8;
    GbcTileAttribute tileAttribute { .attribute = 0};
    uint8_t tileIndex = videoRamBanks[0].at(tileMapOffset + (tileY * 32 + tileX));
    if (cartridge.isGbcCartridge()) {
        tileAttribute.attribute = videoRamBanks[1].at(tileMapOffset + (tileY * 32 + tileX));
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
    if (cartridge.isGbcCartridge()) {
        colorIndex = tileAttribute.backgroundPaletteNumber * 8 + colorIndex * 2;
    }
    return std::make_tuple(colorIndex, tileAttribute.backgroundToOAMPriority == 1);
}

void GbGpu::renderLine()
{
    if (registers.lcdEnable == 0) {
        return;
    }
    for (auto& data : backgroundToOAMPriorityMap) {
        std::fill_n(data.begin(), data.size(), false);
    }
    bool isWindowVisible = false;
    auto& output = display->getBuffer();
    QRgb* line = reinterpret_cast<QRgb*>(output.scanLine(registers.lcdYCoordinate));
    for (uint8_t i = 0; i < 160; i++) {
        line[i] = 0xFFFFFFFF;
        if (cartridge.isGbcCartridge() || registers.backgroundAndWindowPriority == 1) {
            uint8_t x = (i + registers.scrollX) % 256;
            uint8_t y = (registers.lcdYCoordinate + registers.scrollY) % 256;
            size_t tileMapOffset = registers.backgroundTileMapArea == 0 ? 0x1800 : 0x1c00;
            auto [ colorIndex, priority ] = getColorIndexAndPriorityOfBackgroundOrWindow(x, y, tileMapOffset);
            colorIndexMap[i][registers.lcdYCoordinate] = colorIndex & 0b111;
            backgroundToOAMPriorityMap[i][registers.lcdYCoordinate] = priority;
            if (cartridge.isGbcCartridge()) {
                line[i] = gbcPalette->getBackgroundColor(colorIndex);
            } else {
                line[i] = getGbColor(colorIndex, registers.backgroundPaletteData);
            }
        }
        if ((cartridge.isGbcCartridge() || registers.backgroundAndWindowPriority == 1) &&
                (registers.windowEnable == 1 && (registers.windowXPosition - 7) <= i && registers.windowYPosition <= registers.lcdYCoordinate)) {
            uint8_t x = i - registers.windowXPosition + 7;
            uint8_t y = windowLineCounter;
            size_t tileMapOffset = registers.windowTileMapArea == 0 ? 0x1800 : 0x1c00;
            auto [ colorIndex, priority ] = getColorIndexAndPriorityOfBackgroundOrWindow(x, y, tileMapOffset);
            colorIndexMap[i][registers.lcdYCoordinate] = colorIndex & 0b111;
            backgroundToOAMPriorityMap[i][registers.lcdYCoordinate] = priority;
            if (cartridge.isGbcCartridge()) {
                line[i] = gbcPalette->getBackgroundColor(colorIndex);
            } else {
                line[i] = getGbColor(colorIndex, registers.backgroundPaletteData);
            }
            isWindowVisible = true;
        }
    }
    if (isWindowVisible) {
        windowLineCounter++;
    }
    if (registers.spriteEnable == 0) {
        return;
    }
    while (!spriteStack.empty()) {
        uint8_t spriteIndex = spriteStack.top();
        spriteStack.pop();

        uint8_t tileIndex = spriteAttributeTable->at(spriteIndex + 2);
        if (registers.spriteSize == 1) {
            tileIndex &= 0xFE;
        }
        GbcSpriteAttribute spriteAttribute { .attribute = 0 };
        spriteAttribute.attribute = spriteAttributeTable->at(spriteIndex + 3);

        bool isBank1 = cartridge.isGbcCartridge() && (spriteAttribute.tileVideoRamBank == 1);
        const auto& videoRamBank = isBank1 ? videoRamBanks[1] : videoRamBanks[0];

        for (uint8_t i = 0; i < 160; i++) {
            if (backgroundToOAMPriorityMap[i][registers.lcdYCoordinate] && colorIndexMap[i][registers.lcdYCoordinate] > 0) {
                if (cartridge.isGbcCartridge() && registers.backgroundAndWindowPriority == 1) {
                    continue;
                }
                if (!cartridge.isGbcCartridge()) {
                    continue;
                }
            }
            if (spriteAttribute.backgroundToOAMPriority == 1 && colorIndexMap[i][registers.lcdYCoordinate] > 0) {
                if (cartridge.isGbcCartridge() && registers.backgroundAndWindowPriority == 1) {
                    continue;
                }
                if (!cartridge.isGbcCartridge()) {
                    continue;
                }
            }

            int16_t pixelXInTile = i - spriteAttributeTable->at(spriteIndex + 1) + 8;
            if (pixelXInTile >= 8) {
                break;
            }
            if (pixelXInTile < 0) {
                continue;
            }
            int16_t pixelYInTile = registers.lcdYCoordinate - spriteAttributeTable->at(spriteIndex) + 16;
            uint8_t realTileIndex = tileIndex;
            if (registers.spriteSize == 1) {
                if (pixelYInTile < 8 && spriteAttribute.verticalFlip == 1) {
                    realTileIndex += 1;
                } else if (pixelYInTile >= 8 && spriteAttribute.verticalFlip == 0) {
                    realTileIndex += 1;
                }
            }
            pixelYInTile = pixelYInTile & 0x7;

            uint8_t bitPositionInByte = spriteAttribute.horizontalFlip ? (1 << pixelXInTile) : (1 << (7 - pixelXInTile));
            size_t byteIndex = realTileIndex * 16 + (spriteAttribute.verticalFlip ? (7 - pixelYInTile) : pixelYInTile) * 2;
            uint8_t colorIndex = ((videoRamBank.at(byteIndex) & bitPositionInByte) ? 1 : 0) + ((videoRamBank.at(byteIndex + 1) & bitPositionInByte) ? 2 : 0);

            if (colorIndex == 0) {
                continue;
            }
            if (cartridge.isGbcCartridge()) {
                colorIndex = spriteAttribute.gbcPaletteNumber * 8 + colorIndex * 2;
            }
            if (cartridge.isGbcCartridge()) {
                line[i] = gbcPalette->getSpriteColor(colorIndex);
            } else {
                line[i] = getGbColor(colorIndex, spriteAttribute.paletteNumber == 0 ? registers.spritePalette0Data : registers.spritePalette1Data);
            }
        }
    }
}

void GbGpu::scanSprites()
{
    spriteStack = {};
    for (uint8_t i = 0; i < 160; i += 4) {
        int16_t y = spriteAttributeTable->at(i) - 16;
        if (registers.spriteSize == 1) {
            if (static_cast<int16_t>(registers.lcdYCoordinate) >= y && static_cast<int16_t>(registers.lcdYCoordinate) < (y + 16)) {
                spriteStack.push(i);
            }
        } else {
            if (static_cast<int16_t>(registers.lcdYCoordinate) >= y && static_cast<int16_t>(registers.lcdYCoordinate) < (y + 8)) {
                spriteStack.push(i);
            }
        }
        if (spriteStack.size() >= 10) {
            break;
        }
    }
}

}  // namespace qnemu
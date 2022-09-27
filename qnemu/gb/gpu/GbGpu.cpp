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
#include "qnemu/gb/gpu/GbPalette.h"
#include "qnemu/gb/gpu/GbVideoRam.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/Mode.h"
#include "qnemu/gb/gpu/SpriteAttributeTable.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

GbGpu::GbGpu(const GbCartridgeInterface& cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler) :
    cartridge(cartridge),
    display(display),
    interruptHandler(interruptHandler),
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
    if (address >= 0xFF40 && address <= 0xFF45) {
        return true;
    }
    if (address >= 0xFF4A && address <= 0xFF4B) {
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
    if (address == 0xFF4A) {
        return registers.windowYPosition;
    }
    if (address == 0xFF4B) {
        return registers.windowXPosition;
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
    if (address == 0xFF4A) {
        registers.windowYPosition = value;
    }
    if (address == 0xFF4B) {
        registers.windowXPosition = value;
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

    for (auto& subDevice : subDevices) {
        subDevice.get().reset();
    }
}

void GbGpu::addGbPalette(std::unique_ptr<GbPalette> gbPalette)
{
    subDevices.push_back(*gbPalette);
    this->gbPalette = std::move(gbPalette);
}

void GbGpu::addGbcPalette(std::unique_ptr<GbcPalette> gbcPalette)
{
    subDevices.push_back(*gbcPalette);
    this->gbcPalette = std::move(gbcPalette);
}

void GbGpu::addSpriteAttributeTable(std::unique_ptr<SpriteAttributeTable> spriteAttributeTable)
{
    subDevices.push_back(*spriteAttributeTable);
    this->spriteAttributeTable = std::move(spriteAttributeTable);
}

void GbGpu::addGbVideoRam(std::unique_ptr<GbVideoRam> gbVideoRam)
{
    subDevices.push_back(*gbVideoRam);
    this->gbVideoRam = std::move(gbVideoRam);
}

void GbGpu::checklcdYCoordinate()
{
    if (registers.lcdYCoordinate == registers.lcdYCoordinateCompare) {
        registers.coincidenceFlag = 1;
        if (registers.lyCoincidenceInterrupt) {
            interruptHandler->requestLcdInterrupt();
        }
    }
    else {
        registers.coincidenceFlag = 0;
    }
}

std::tuple<uint16_t, bool> GbGpu::getColorIndexAndPriorityOfBackgroundOrWindow(uint8_t x, uint8_t y, size_t tileMapOffset) const
{
    const uint8_t tileX = x / 8, tileY = y / 8;
    GbcTileAttribute tileAttribute { .attribute = 0};
    const uint8_t tileIndex = gbVideoRam->getBank(0).at(tileMapOffset + (tileY * 32 + tileX));
    if (cartridge.isGbcCartridge()) {
        tileAttribute.attribute = gbVideoRam->getBank(1).at(tileMapOffset + (tileY * 32 + tileX));
    }

    const bool isBank1 = cartridge.isGbcCartridge() && (tileAttribute.tileVideoRamBank == 1);
    const uint8_t pixelXinTile = x % 8, pixelYinTile = y % 8;
    const uint8_t bitPositionInByte = tileAttribute.horizontalFlip ? 1 << pixelXinTile : 1 << (7 - pixelXinTile);
    size_t byteIndex = tileIndex * 16 + (tileAttribute.verticalFlip ? (7 - pixelYinTile) : pixelYinTile) * 2;
    if (tileIndex < 128) {
        byteIndex += registers.backgroundAndWindowTileDataArea == 1 ? 0 : 0x1000;
    }
    const auto& videoRamBank = isBank1 ? gbVideoRam->getBank(1) : gbVideoRam->getBank(0);
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
            const uint8_t x = (i + registers.scrollX) % 256;
            const uint8_t y = (registers.lcdYCoordinate + registers.scrollY) % 256;
            const size_t tileMapOffset = registers.backgroundTileMapArea == 0 ? 0x1800 : 0x1c00;
            auto [ colorIndex, priority ] = getColorIndexAndPriorityOfBackgroundOrWindow(x, y, tileMapOffset);
            colorIndexMap[i][registers.lcdYCoordinate] = colorIndex & 0b111;
            backgroundToOAMPriorityMap[i][registers.lcdYCoordinate] = priority;
            if (cartridge.isGbcCartridge()) {
                line[i] = gbcPalette->getBackgroundColor(colorIndex);
            } else {
                line[i] = gbPalette->getBackgroundColor(colorIndex);
            }
        }
        if ((cartridge.isGbcCartridge() || registers.backgroundAndWindowPriority == 1) &&
                (registers.windowEnable == 1 && (registers.windowXPosition - 7) <= i && registers.windowYPosition <= registers.lcdYCoordinate)) {
            const uint8_t x = i - registers.windowXPosition + 7;
            const uint8_t y = windowLineCounter;
            const size_t tileMapOffset = registers.windowTileMapArea == 0 ? 0x1800 : 0x1c00;
            auto [ colorIndex, priority ] = getColorIndexAndPriorityOfBackgroundOrWindow(x, y, tileMapOffset);
            colorIndexMap[i][registers.lcdYCoordinate] = colorIndex & 0b111;
            backgroundToOAMPriorityMap[i][registers.lcdYCoordinate] = priority;
            if (cartridge.isGbcCartridge()) {
                line[i] = gbcPalette->getBackgroundColor(colorIndex);
            } else {
                line[i] = gbPalette->getBackgroundColor(colorIndex);
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
        const uint8_t spriteIndex = spriteStack.top();
        spriteStack.pop();

        uint8_t tileIndex = spriteAttributeTable->at(spriteIndex + 2);
        if (registers.spriteSize == 1) {
            tileIndex &= 0xFE;
        }
        GbcSpriteAttribute spriteAttribute { .attribute = 0 };
        spriteAttribute.attribute = spriteAttributeTable->at(spriteIndex + 3);

        const bool isBank1 = cartridge.isGbcCartridge() && (spriteAttribute.tileVideoRamBank == 1);
        const auto& videoRamBank = isBank1 ? gbVideoRam->getBank(1) : gbVideoRam->getBank(0);

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

            const int16_t pixelXInTile = i - spriteAttributeTable->at(spriteIndex + 1) + 8;
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

            const uint8_t bitPositionInByte = spriteAttribute.horizontalFlip ? (1 << pixelXInTile) : (1 << (7 - pixelXInTile));
            const size_t byteIndex = realTileIndex * 16 + (spriteAttribute.verticalFlip ? (7 - pixelYInTile) : pixelYInTile) * 2;
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
                line[i] = gbPalette->getSpriteColor(colorIndex, spriteAttribute.paletteNumber);
            }
        }
    }
}

void GbGpu::scanSprites()
{
    spriteStack = {};
    for (uint8_t i = 0; i < 160; i += 4) {
        const int16_t y = spriteAttributeTable->at(i) - 16;
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
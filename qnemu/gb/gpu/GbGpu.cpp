/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <cstring>

#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/gpu/Mode.h"

namespace qnemu
{

GbGpu::GbGpu(std::shared_ptr<GbInterruptHandler> interruptHandler) :
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

void GbGpu::scanSprites()
{
    
}

}  // namespace qnemu
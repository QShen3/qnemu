/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

#include "tests/rom/GbTestSystem.h"

namespace qnemuTest
{

namespace
{
constexpr uint16_t serialDataAddress = 0xFF01;
constexpr uint16_t serialControlAddress = 0xFF02;
constexpr uint16_t dividerAddress = 0xFF04;
constexpr uint16_t timaAddress = 0xFF05;
constexpr uint16_t tmaAddress = 0xFF06;
constexpr uint16_t tacAddress = 0xFF07;
constexpr uint16_t interruptFlagAddress = 0xFF0F;
constexpr uint16_t lcdcAddress = 0xFF40;
constexpr uint16_t statAddress = 0xFF41;
constexpr uint16_t lyAddress = 0xFF44;
constexpr uint16_t lycAddress = 0xFF45;
constexpr uint16_t key0Address = 0xFF4C;
constexpr uint16_t interruptEnableAddress = 0xFFFF;
}  // namespace

bool GbTestSystem::loadRom(const std::string& path, bool cgb, bool enablePpu)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    const std::streamsize size = file.tellg();
    if (size < 0x150) {
        return false;
    }

    file.seekg(0, std::ios::beg);
    rom_.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(rom_.data()), size)) {
        return false;
    }

    cgb_ = cgb;
    const uint8_t cartridgeType = rom_[0x147];
    mbc1_ = cartridgeType >= 0x01 && cartridgeType <= 0x03;

    ppu.reset();
    display.reset();
    reset();

    if (enablePpu) {
        display = std::make_shared<MemoryDisplay>();
        ppu = std::make_unique<qnemu::GbPpu>(cgb, display, interruptHandler);
        ppu->setExternalRead([this](uint16_t address) {
            return read(address);
        });
    }
    return true;
}

void GbTestSystem::reset()
{
    memory_.fill(0);
    ramEnabled_ = false;
    romBankLow_ = 1;
    ramBank_ = 0;
    bankingMode_ = 0;

    divider_ = 0xAB00;
    tima_ = 0;
    tma_ = 0;
    tac_ = 0xF8;
    previousTimerBit_ = 0;
    timaOverflow_ = false;
    overflowCycles_ = 0;

    interruptHandler->reset();

    lcdc_ = 0x91;
    stat_ = 0x80;
    ly_ = 0;
    lyc_ = 0;
    ppuDivider_ = 0;

    if (ppu) {
        ppu->reset();
    }

    serialData_ = 0;
    serialControl_ = 0x00;
    serialOutput_.clear();
    nr52_ = 0x80;

    doubleSpeed_ = false;
    speedSwitchArmed_ = false;
    speedRemainder_ = 0;

    cycles_ = 0;
}

uint8_t GbTestSystem::read(uint16_t address)
{
    if (address < 0x8000) {
        const uint32_t offset = romOffset(address);
        return offset < rom_.size() ? rom_[offset] : 0xFF;
    }

    if (address >= 0xE000 && address <= 0xFDFF) {
        address = static_cast<uint16_t>(address - 0x2000);
    }

    if (ppu
        && ((address >= 0x8000 && address <= 0x9FFF)
            || (address >= 0xFE00 && address <= 0xFE9F)
            || (address >= 0xFF40 && address <= 0xFF4B)
            || address == 0xFF4F
            || (address >= 0xFF51 && address <= 0xFF55)
            || (address >= 0xFF68 && address <= 0xFF6C))) {
        return ppu->read(address);
    }

    if (address == 0xFF00) {
        return 0xFF;
    }
    if (address == serialDataAddress) {
        return serialData_;
    }
    if (address == serialControlAddress) {
        return serialControl_;
    }
    if (address == dividerAddress) {
        return static_cast<uint8_t>(divider_ >> 8);
    }
    if (address == timaAddress) {
        return tima_;
    }
    if (address == tmaAddress) {
        return tma_;
    }
    if (address == tacAddress) {
        return static_cast<uint8_t>(tac_ | 0xF8);
    }
    if (address == interruptFlagAddress) {
        return interruptHandler->read(address);
    }
    if (address == lcdcAddress) {
        return lcdc_;
    }
    if (address == statAddress) {
        return stat_;
    }
    if (address == lyAddress) {
        return ly_;
    }
    if (address == lycAddress) {
        return lyc_;
    }
    if (address == key0Address) {
        return cgb_ ? 0x80 : 0x00;
    }
    if (address == 0xFF4D) {
        return static_cast<uint8_t>(0x7E
            | (doubleSpeed_ ? 0x80 : 0x00)
            | (speedSwitchArmed_ ? 0x01 : 0x00));
    }
    if (address == 0xFF26) {
        return nr52_;
    }
    if (address >= 0xFF10 && address <= 0xFF3F) {
        return memory_[address];
    }
    if (address == interruptEnableAddress) {
        return interruptHandler->read(address);
    }

    if (address >= 0xFF00 && address < 0xFF80) {
        return 0xFF;
    }
    return memory_[address];
}

void GbTestSystem::write(uint16_t address, uint8_t value)
{
    if (address < 0x8000) {
        if (!mbc1_) {
            return;
        }
        if (address < 0x2000) {
            ramEnabled_ = (value & 0x0F) == 0x0A;
        } else if (address < 0x4000) {
            romBankLow_ = static_cast<uint8_t>(value & 0x1F);
            if (romBankLow_ == 0) {
                romBankLow_ = 1;
            }
        } else if (address < 0x6000) {
            ramBank_ = static_cast<uint8_t>(value & 0x03);
        } else {
            bankingMode_ = static_cast<uint8_t>(value & 0x01);
        }
        return;
    }

    if (address >= 0xE000 && address <= 0xFDFF) {
        address = static_cast<uint16_t>(address - 0x2000);
    }

    if (ppu
        && ((address >= 0x8000 && address <= 0x9FFF)
            || (address >= 0xFE00 && address <= 0xFE9F)
            || (address >= 0xFF40 && address <= 0xFF4B)
            || address == 0xFF4F
            || (address >= 0xFF51 && address <= 0xFF55)
            || (address >= 0xFF68 && address <= 0xFF6C))) {
        ppu->write(address, value);
        return;
    }

    if (address == serialDataAddress) {
        serialData_ = value;
        return;
    }
    if (address == serialControlAddress) {
        if ((value & 0x80) != 0) {
            serialOutput_.push_back(static_cast<char>(serialData_));
            serialControl_ = 0x01;
        } else {
            serialControl_ = value;
        }
        return;
    }
    if (address == dividerAddress) {
        const bool fallingEdge = (tac_ & 0x04) != 0 && timerClockBit() != 0;
        divider_ = 0;
        previousTimerBit_ = 0;
        if (fallingEdge) {
            incrementTimerCounter();
        }
        return;
    }
    if (address == timaAddress) {
        tima_ = value;
        return;
    }
    if (address == tmaAddress) {
        tma_ = value;
        return;
    }
    if (address == tacAddress) {
        writeTimerControl(value);
        return;
    }
    if (address == interruptFlagAddress) {
        interruptHandler->write(address, value);
        return;
    }
    if (address == lcdcAddress) {
        lcdc_ = value;
        if ((lcdc_ & 0x80) == 0) {
            ly_ = 0;
            ppuDivider_ = 0;
        }
        return;
    }
    if (address == statAddress) {
        stat_ = static_cast<uint8_t>((stat_ & 0x04) | (value & 0x78) | 0x80);
        return;
    }
    if (address == lyAddress) {
        ly_ = 0;
        return;
    }
    if (address == lycAddress) {
        lyc_ = value;
        return;
    }
    if (address == 0xFF4D) {
        if ((value & 0x01) != 0) {
            speedSwitchArmed_ = true;
        }
        return;
    }
    if (address == 0xFF26) {
        nr52_ = static_cast<uint8_t>(value & 0x80);
        return;
    }
    if (address >= 0xFF10 && address <= 0xFF3F) {
        memory_[address] = value;
        return;
    }
    if (address == interruptEnableAddress) {
        interruptHandler->write(address, value);
        return;
    }

    if (address >= 0x8000 && address <= 0xFDFF) {
        memory_[address] = value;
        return;
    }
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        memory_[address] = value;
        return;
    }
    if (address >= 0xFF80 && address <= 0xFFFE) {
        memory_[address] = value;
    }
}

void GbTestSystem::tick(uint8_t cycles)
{
    uint8_t peripheralCycles = cycles;
    if (doubleSpeed_) {
        const uint8_t total = static_cast<uint8_t>(cycles + speedRemainder_);
        peripheralCycles = static_cast<uint8_t>(total / 2);
        speedRemainder_ = static_cast<uint8_t>(total % 2);
    } else {
        speedRemainder_ = 0;
    }

    for (uint8_t cycle = 0; cycle < peripheralCycles; ++cycle) {
        ++divider_;
        timerTick();
        if (ppu) {
            ppu->step();
        } else {
            ppuTick();
        }
        ++cycles_;
    }
}

bool GbTestSystem::onStop()
{
    if (speedSwitchArmed_) {
        speedSwitchArmed_ = false;
        doubleSpeed_ = !doubleSpeed_;
        speedRemainder_ = 0;
        return false;
    }
    return true;
}

void GbTestSystem::timerTick()
{
    if (timaOverflow_) {
        if (overflowCycles_ > 0) {
            --overflowCycles_;
        }
        if (overflowCycles_ == 0) {
            tima_ = tma_;
            timaOverflow_ = false;
            interruptHandler->requestTimerInterrupt();
        }
    }

    if ((tac_ & 0x04) == 0) {
        previousTimerBit_ = 0;
        return;
    }

    const uint8_t bit = timerClockBit();
    if (previousTimerBit_ != 0 && bit == 0) {
        incrementTimerCounter();
    }
    previousTimerBit_ = bit;
}

void GbTestSystem::incrementTimerCounter()
{
    if (tima_ == 0xFF) {
        tima_ = 0;
        timaOverflow_ = true;
        overflowCycles_ = 4;
    } else {
        ++tima_;
    }
}

void GbTestSystem::writeTimerControl(uint8_t value)
{
    const bool wasEnabled = (tac_ & 0x04) != 0;
    const uint8_t oldBit = timerClockBit();
    tac_ = value;
    const bool isEnabled = (tac_ & 0x04) != 0;
    const uint8_t newBit = timerClockBit();

    if (wasEnabled && oldBit != 0 && (!isEnabled || newBit == 0)) {
        incrementTimerCounter();
    }

    previousTimerBit_ = isEnabled ? newBit : 0;
}

void GbTestSystem::ppuTick()
{
    if ((lcdc_ & 0x80) == 0) {
        ly_ = 0;
        ppuDivider_ = 0;
        return;
    }

    ++ppuDivider_;
    if (ppuDivider_ >= 456) {
        ppuDivider_ = static_cast<uint16_t>(ppuDivider_ - 456);
        ++ly_;
        if (ly_ == 144) {
            interruptHandler->requestVBlankInterrupt();
        }
        if (ly_ >= 154) {
            ly_ = 0;
        }
    }
}

std::string GbTestSystem::screenText()
{
    std::string text;
    for (uint16_t row = 0; row < 32; ++row) {
        std::string line;
        for (uint16_t column = 0; column < 32; ++column) {
            const uint16_t address = static_cast<uint16_t>(0x9800 + row * 32 + column);
            const uint8_t tile = ppu ? ppu->read(address) : memory_[address];
            line.push_back(tile >= 0x20 && tile < 0x7F ? static_cast<char>(tile) : ' ');
        }
        while (!line.empty() && line.back() == ' ') {
            line.pop_back();
        }
        if (!line.empty()) {
            text += line;
            text.push_back('\n');
        }
    }
    return text;
}

uint8_t GbTestSystem::timerClockBit() const
{
    uint8_t bit = 0;
    switch (tac_ & 0x03) {
        case 0: bit = 9; break;
        case 1: bit = 3; break;
        case 2: bit = 5; break;
        default: bit = 7; break;
    }
    return static_cast<uint8_t>((divider_ >> bit) & 0x01);
}

void GbTestSystem::updateTimerClockBit()
{
    previousTimerBit_ = (tac_ & 0x04) != 0 ? timerClockBit() : 0;
}

uint32_t GbTestSystem::romOffset(uint16_t address) const
{
    uint16_t bank = 0;
    if (address < 0x4000) {
        if (mbc1_ && bankingMode_ == 1) {
            bank = static_cast<uint16_t>((ramBank_ & 0x03) << 5);
        }
    } else {
        bank = mbc1_
            ? static_cast<uint16_t>((romBankLow_ | ((ramBank_ & 0x03) << 5)) & 0x7F)
            : 1;
        if (bank == 0) {
            bank = 1;
        }
    }

    return static_cast<uint32_t>(bank) * 0x4000 + (address & 0x3FFF);
}

}  // namespace qnemuTest

/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "tests/rom/GbTestSystem.h"
#include "tests/support/NullSound.h"

namespace qnemuTest
{

namespace
{
constexpr uint16_t serialDataAddress = 0xFF01;
constexpr uint16_t serialControlAddress = 0xFF02;
constexpr uint16_t dividerAddress = 0xFF04;
constexpr uint16_t tacAddress = 0xFF07;
constexpr uint16_t interruptFlagAddress = 0xFF0F;
constexpr uint16_t lcdcAddress = 0xFF40;
constexpr uint16_t statAddress = 0xFF41;
constexpr uint16_t lyAddress = 0xFF44;
constexpr uint16_t lycAddress = 0xFF45;
constexpr uint16_t key0Address = 0xFF4C;
constexpr uint16_t interruptEnableAddress = 0xFFFF;
}  // namespace

bool GbTestSystem::loadRom(const std::string& path, bool cgb, bool enablePpu,
    bool enableApu)
{
    try {
        cartridge = std::make_unique<qnemu::GbCartridge>(mbcFactory);
        cartridge->load(path.c_str());
    } catch (const std::exception&) {
        cartridge.reset();
        return false;
    }

    cgb_ = cgb;

    ppu.reset();
    display.reset();
    apu.reset();
    timer = std::make_unique<qnemu::GbTimer>(interruptHandler);
    reset();

    if (enablePpu) {
        display = std::make_shared<MemoryDisplay>();
        const bool ppuCgb = cgb && cartridge->isGbcCartridge();
        ppu = std::make_unique<qnemu::GbPpu>(ppuCgb, display, interruptHandler);
        ppu->setExternalRead([this](uint16_t address) {
            return read(address);
        });
    }

    if (enableApu) {
        std::array<std::unique_ptr<qnemu::GbChannelInterface>, 4> channels {
            std::make_unique<qnemu::GbChannel1>(),
            std::make_unique<qnemu::GbChannel2>(),
            std::make_unique<qnemu::GbChannel3>(),
            std::make_unique<qnemu::GbChannel4>(),
        };
        apu = std::make_unique<qnemu::GbApu>(
            std::make_unique<NullSound>(), std::move(channels), cgb);
    }
    return true;
}

void GbTestSystem::reset()
{
    memory_.fill(0);

    if (timer) {
        timer->setDoubleSpeed(false);
        timer->reset();
    }

    interruptHandler->reset();

    if (apu) {
        apu->reset();
    }

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
    if (address >= 0xE000 && address <= 0xFDFF) {
        address = static_cast<uint16_t>(address - 0x2000);
    }

    if (address < 0x8000 || (address >= 0xA000 && address <= 0xBFFF)) {
        return cartridge->read(address);
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

    if (apu && address >= 0xFF10 && address <= 0xFF3F) {
        return apu->read(address);
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
    if (address >= dividerAddress && address <= tacAddress) {
        return timer->read(address);
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
    if (address >= 0xE000 && address <= 0xFDFF) {
        address = static_cast<uint16_t>(address - 0x2000);
    }

    if (address < 0x8000 || (address >= 0xA000 && address <= 0xBFFF)) {
        cartridge->write(address, value);
        return;
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

    if (apu && address == 0xFF26) {
        const bool wasPowered = apu->isPowered();
        apu->write(address, value);
        if (!wasPowered && apu->isPowered() && timer->isDivApuBitHigh()) {
            apu->skipNextDivApuTick();
        }
        return;
    }

    if (apu && address >= 0xFF10 && address <= 0xFF3F) {
        apu->write(address, value);
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
    if (address >= dividerAddress && address <= tacAddress) {
        timer->write(address, value);
        if (apu && timer->takeDivApuTick()) {
            apu->divApuTick();
        }
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
        if (apu) {
            apu->step();
        }
        timer->step();
        if (apu && timer->takeDivApuTick()) {
            apu->divApuTick();
        }
        if (cartridge) {
            cartridge->step();
        }
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
        timer->setDoubleSpeed(doubleSpeed_);
        return false;
    }
    return true;
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

}  // namespace qnemuTest

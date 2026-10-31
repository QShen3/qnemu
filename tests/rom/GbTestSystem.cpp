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
#include "qnemu/gb/boot/GbCgbBootRom.h"
#include "qnemu/gb/gpu/CgbCompatibilityPalette.h"
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
    return finishLoad(cgb, enablePpu, enableApu);
}

bool GbTestSystem::loadRomData(const std::vector<uint8_t>& romData, bool cgb,
    bool enablePpu, bool enableApu)
{
    try {
        cartridge = std::make_unique<qnemu::GbCartridge>(mbcFactory);
        cartridge->load(romData);
    } catch (const std::exception&) {
        cartridge.reset();
        return false;
    }
    return finishLoad(cgb, enablePpu, enableApu);
}

void GbTestSystem::enableBootRom()
{
    bootRom_ = qnemu::cgbBootRom();
    bootRomEnabled_ = true;
}

bool GbTestSystem::finishLoad(bool cgb, bool enablePpu, bool enableApu)
{
    cgb_ = cgb;
    cgbHardware_ = cgb;
    bootRomActive_ = bootRomEnabled_ && cgb;

    ppu.reset();
    display.reset();
    apu.reset();
    timer = std::make_unique<qnemu::GbTimer>(interruptHandler);
    reset();

    if (enablePpu) {
        display = std::make_shared<MemoryDisplay>();
        const auto header = cartridge->cartridgeHeader();
        const bool ppuCgb = cgb && (bootRomActive_ || header.cgb);
        ppu = std::make_unique<qnemu::GbPpu>(ppuCgb, display, interruptHandler);
        ppu->setOamBugEnabled(!cgb);
        if (!bootRomActive_ && cgb && !header.cgb) {
            ppu->setDmgCompatibilityPalette(qnemu::cgbCompatibilityPalette({
                header.titleBytes,
                header.oldLicensee,
                header.newLicensee,
            }));
        } else {
            ppu->clearDmgCompatibilityPalette();
        }
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

void GbTestSystem::applyBootState(BootModel model)
{
    uint16_t divider = 0xABCD;
    lcdc_ = 0x91;
    stat_ = 0x80;
    scy_ = 0;
    scx_ = 0;
    ly_ = 0x0A;
    lyc_ = 0;
    bgp_ = 0xFC;
    obp0_ = 0xFF;
    obp1_ = 0xFF;
    wy_ = 0;
    wx_ = 0;
    joypad_ = 0xCF;
    nr52_ = 0xF1;

    switch (model) {
    case BootModel::dmg0:
        divider = 0x1831;
        stat_ = 0x83;
        ly_ = 0x01;
        break;
    case BootModel::dmgAbc:
    case BootModel::mgb:
        break;
    case BootModel::sgb:
        divider = 0xD861;
        lcdc_ = 0xFF;
        stat_ = 0xFF;
        ly_ = 0x00;
        lyc_ = 0x00;
        joypad_ = 0xFF;
        nr52_ = 0xF0;
        break;
    case BootModel::sgb2:
        divider = 0xD851;
        lcdc_ = 0xFF;
        stat_ = 0xFF;
        ly_ = 0x00;
        lyc_ = 0x00;
        joypad_ = 0xFF;
        nr52_ = 0xF0;
        break;
    }

    // The headless bus has no real PPU.  Keep the boot LY value stable for
    // boot-state checks instead of running the tiny VBlank stub.
    freezeLy_ = true;

    timer->setDivider(divider);
    serialData_ = 0;
    serialControl_ = 0x7E;
    serialTimingEnabled_ = true;
    serialCyclesRemaining_ = 0;
    serialClockPhase_ = 50;

    // Post-boot APU register values, excluding the model-specific NR52.
    static constexpr uint8_t apuBoot[0x18] = {
        0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00,
        0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
        0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF,
    };
    for (std::size_t index = 0; index < sizeof(apuBoot); ++index) {
        memory_[0xFF10 + index] = apuBoot[index];
    }
    for (uint16_t address = 0xFF28; address <= 0xFF3F; ++address) {
        memory_[address] = 0xFF;
    }
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
    scy_ = 0xFF;
    scx_ = 0xFF;
    ly_ = 0;
    lyc_ = 0;
    bgp_ = 0xFF;
    obp0_ = 0xFF;
    obp1_ = 0xFF;
    wy_ = 0xFF;
    wx_ = 0xFF;
    joypad_ = 0xCF;
    ppuDivider_ = 0;
    freezeLy_ = false;
    serialTimingEnabled_ = false;
    serialCyclesRemaining_ = 0;
    serialClockPhase_ = 50;

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

    if (bootRomActive_ && bootRom_.size() >= 0x0900
        && (address <= 0x00FF || (address >= 0x0200 && address <= 0x08FF))) {
        return bootRom_[address];
    }

    if (address < 0x8000 || (address >= 0xA000 && address <= 0xBFFF)) {
        return cartridge->read(address);
    }

    if (ppu
        && ((address >= 0x8000 && address <= 0x9FFF)
            || (address >= 0xFE00 && address <= 0xFEFF)
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
        return static_cast<uint8_t>(0xCF | (joypad_ & 0x30));
    }
    if (address == serialDataAddress) {
        return serialData_;
    }
    if (address == serialControlAddress) {
        // Bits 1-6 are unused and read as 1; bit 7 is the transfer flag and
        // bit 0 is the clock select.
        return static_cast<uint8_t>(0x7E | (serialControl_ & 0x81));
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
    if (address == 0xFF42) {
        return scy_;
    }
    if (address == 0xFF43) {
        return scx_;
    }
    if (address == lyAddress) {
        return ly_;
    }
    if (address == lycAddress) {
        return lyc_;
    }
    if (address == 0xFF47) {
        return bgp_;
    }
    if (address == 0xFF48) {
        return obp0_;
    }
    if (address == 0xFF49) {
        return obp1_;
    }
    if (address == 0xFF4A) {
        return wy_;
    }
    if (address == 0xFF4B) {
        return wx_;
    }
    if (address == key0Address) {
        return cgb_ ? 0x80 : 0xFF;
    }
    if (address == 0xFF50) {
        return 0xFF;
    }
    if (address == 0xFF70) {
        return cgb_ ? static_cast<uint8_t>(0xF8 | (memory_[address] & 0x07)) : 0xFF;
    }
    if (address >= 0xFF72 && address <= 0xFF75) {
        if (!cgbHardware_ || (address == 0xFF74 && !cgb_)) {
            return 0xFF;
        }
        return address == 0xFF75
            ? static_cast<uint8_t>(0x8F | (memory_[address] & 0x70))
            : memory_[address];
    }
    if (address == 0xFF4D) {
        if (!cgb_) {
            return 0xFF;
        }
        return static_cast<uint8_t>(0x7E
            | (doubleSpeed_ ? 0x80 : 0x00)
            | (speedSwitchArmed_ ? 0x01 : 0x00));
    }
    if (address == 0xFF26) {
        return nr52_;
    }
    if (address == 0xFF76 || address == 0xFF77) {
        if (apu) {
            return apu->read(address);
        }
        return cgbHardware_ ? 0x00 : 0xFF;
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

    if (address == 0xFF50) {
        bootRomActive_ = false;
        return;
    }

    if (address == key0Address) {
        if (cgb_ && (value & 0x04) != 0) {
            cgb_ = false;
            if (ppu) {
                ppu->setDmgMode(true);
                ppu->enableDmgCompatibility();
            }
        }
        return;
    }

    if (address == 0xFF70) {
        memory_[address] = value;
        return;
    }
    if (address >= 0xFF72 && address <= 0xFF75) {
        if (cgbHardware_) {
            memory_[address] = address == 0xFF75
                ? static_cast<uint8_t>(value & 0x70)
                : value;
        }
        return;
    }

    if (address == 0xFF00) {
        joypad_ = static_cast<uint8_t>(value & 0x30);
        return;
    }

    if (address < 0x8000 || (address >= 0xA000 && address <= 0xBFFF)) {
        cartridge->write(address, value);
        return;
    }

    if (ppu
        && ((address >= 0x8000 && address <= 0x9FFF)
            || (address >= 0xFE00 && address <= 0xFEFF)
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
        if (!serialTimingEnabled_) {
            if ((value & 0x80) != 0) {
                serialOutput_.push_back(static_cast<char>(serialData_));
                serialControl_ = 0x01;
            } else {
                serialControl_ = value;
            }
            return;
        }

        if ((value & 0x80) != 0) {
            // Internal clock: the first bit period starts at the next
            // free-running clock edge, and completion is seven periods later.
            const uint16_t now = static_cast<uint16_t>(cycles_ % 512);
            uint16_t delay = static_cast<uint16_t>(
                (serialClockPhase_ + 512 - now) % 512);
            if (delay == 0) {
                delay = 512;
            }
            serialCyclesRemaining_ = static_cast<uint16_t>(delay + 7 * 512);
            serialControl_ = static_cast<uint8_t>(value & 0x01);
            serialControl_ |= 0x80;
        } else {
            serialControl_ = static_cast<uint8_t>(value & 0x01);
            serialCyclesRemaining_ = 0;
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
    if (address == 0xFF42) {
        scy_ = value;
        return;
    }
    if (address == 0xFF43) {
        scx_ = value;
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
    if (address == 0xFF47) {
        bgp_ = value;
        return;
    }
    if (address == 0xFF48) {
        obp0_ = value;
        return;
    }
    if (address == 0xFF49) {
        obp1_ = value;
        return;
    }
    if (address == 0xFF4A) {
        wy_ = value;
        return;
    }
    if (address == 0xFF4B) {
        wx_ = value;
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

        if (serialTimingEnabled_ && serialCyclesRemaining_ > 0) {
            --serialCyclesRemaining_;
            if (serialCyclesRemaining_ == 0) {
                serialControl_ = static_cast<uint8_t>(serialControl_ & 0x01);
                serialOutput_.push_back(static_cast<char>(serialData_));
                interruptHandler->requestSerialInterrupt();
            }
        }

        ++cycles_;
    }
}

void GbTestSystem::triggerOamBugWrite(uint16_t address, int rowOffset)
{
    if (ppu) {
        ppu->triggerOamBugWrite(address, rowOffset);
    }
}

void GbTestSystem::triggerOamBugRead(uint16_t address)
{
    if (ppu) {
        ppu->triggerOamBugRead(address);
    }
}

void GbTestSystem::triggerOamBugReadIncrease(uint16_t address)
{
    if (ppu) {
        ppu->triggerOamBugReadIncrease(address);
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
    if (freezeLy_) {
        return;
    }
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

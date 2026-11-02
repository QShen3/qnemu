/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbMbc6.h"

namespace qnemu
{

GbMbc6::GbMbc6(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(std::move(romBanks)),
    ramBanks(std::move(ramBanks)),
    type(type),
    flash(flashSize, 0xFF)
{
    hiddenFlash.fill(0xFF);
    GbMbc6::reset();
}

uint8_t GbMbc6::read(uint16_t address) const
{
    if (address <= MemoryRomBank00End) {
        if (romBanks.empty()) {
            return 0xFF;
        }
        return romBanks.at(0).at(address);
    }

    if (address <= MemoryRomBank01End) {
        const bool windowB = address >= 0x6000;
        const bool flashSelected = windowB ? flashSelectedB : flashSelectedA;
        if (flashSelected) {
            if (!flashEnabled) {
                return 0xFF;
            }
            return readFlash(flashAddress(address));
        }
        return readRomWindow(address, windowB ? romBankB : romBankA);
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled) {
            return 0xFF;
        }
        const bool windowB = address >= 0xB000;
        return readRamWindow(address, windowB ? ramBankB : ramBankA);
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbMbc6::write(uint16_t address, const uint8_t value)
{
    if (address <= 0x03FF) {
        ramEnabled = (value & 0x0F) == 0x0A;
        return;
    }
    if (address <= 0x07FF) {
        ramBankA = static_cast<uint8_t>(value & 0x07);
        return;
    }
    if (address <= 0x0BFF) {
        ramBankB = static_cast<uint8_t>(value & 0x07);
        return;
    }
    if (address <= 0x0FFF) {
        flashEnabled = (value & 0x01) != 0;
        return;
    }
    if (address <= 0x1FFF) {
        flashWriteEnabled = (value & 0x01) != 0;
        return;
    }
    if (address <= 0x27FF) {
        romBankA = static_cast<uint8_t>(value & 0x7F);
        return;
    }
    if (address <= 0x2FFF) {
        flashSelectedA = (value & 0x08) != 0;
        return;
    }
    if (address <= 0x37FF) {
        romBankB = static_cast<uint8_t>(value & 0x7F);
        return;
    }
    if (address <= 0x3FFF) {
        flashSelectedB = (value & 0x08) != 0;
        return;
    }

    if (address <= MemoryRomBank01End) {
        const bool windowB = address >= 0x6000;
        const bool flashSelected = windowB ? flashSelectedB : flashSelectedA;
        if (flashSelected && flashEnabled) {
            writeFlash(flashAddress(address), value);
        }
        return;
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled) {
            return;
        }
        const bool windowB = address >= 0xB000;
        writeRamWindow(address, windowB ? ramBankB : ramBankA, value);
        return;
    }

    assert(false && "Wrong address");
}

void GbMbc6::reset()
{
    ramEnabled = false;
    ramBankA = 0;
    ramBankB = 0;
    flashEnabled = false;
    flashWriteEnabled = false;
    romBankA = 2;
    romBankB = 3;
    flashSelectedA = false;
    flashSelectedB = false;
    resetFlashCommand();
}

bool GbMbc6::hasBattery() const
{
    return type == 0x20;
}

std::size_t GbMbc6::ramSize() const
{
    return ramBanks.size() * RamBankSize;
}

std::vector<uint8_t> GbMbc6::dumpRam() const
{
    std::vector<uint8_t> data;
    data.reserve(ramSize());
    for (const auto& bank : ramBanks) {
        data.insert(data.end(), bank.begin(), bank.end());
    }
    return data;
}

void GbMbc6::loadRam(const std::vector<uint8_t>& data)
{
    std::size_t offset = 0;
    for (auto& bank : ramBanks) {
        for (auto& byte : bank) {
            byte = offset < data.size() ? data.at(offset++) : 0xFF;
        }
    }
}

uint8_t GbMbc6::readRomWindow(uint16_t address, uint8_t bank) const
{
    if (romBanks.empty()) {
        return 0xFF;
    }

    const uint8_t halfBank = static_cast<uint8_t>(bank & 0x7F);
    const std::size_t romBank = static_cast<std::size_t>(halfBank >> 1)
        & (romBanks.size() - 1);
    const std::size_t offset = static_cast<std::size_t>(halfBank & 0x01) * 0x2000
        + (address & 0x1FFF);
    return romBanks.at(romBank).at(offset);
}

uint8_t GbMbc6::readRamWindow(uint16_t address, uint8_t bank) const
{
    if (ramBanks.empty()) {
        return 0xFF;
    }

    const uint8_t quarterBank = static_cast<uint8_t>(bank & 0x07);
    const std::size_t ramBank = static_cast<std::size_t>(quarterBank >> 1)
        & (ramBanks.size() - 1);
    const std::size_t offset = static_cast<std::size_t>(quarterBank & 0x01) * 0x1000
        + (address & 0x0FFF);
    return ramBanks.at(ramBank).at(offset);
}

void GbMbc6::writeRamWindow(uint16_t address, uint8_t bank, uint8_t value)
{
    if (ramBanks.empty()) {
        return;
    }

    const uint8_t quarterBank = static_cast<uint8_t>(bank & 0x07);
    const std::size_t ramBank = static_cast<std::size_t>(quarterBank >> 1)
        & (ramBanks.size() - 1);
    const std::size_t offset = static_cast<std::size_t>(quarterBank & 0x01) * 0x1000
        + (address & 0x0FFF);
    ramBanks.at(ramBank).at(offset) = value;
}

uint32_t GbMbc6::flashAddress(uint16_t address) const
{
    const bool windowB = address >= 0x6000;
    const uint8_t bank = windowB ? romBankB : romBankA;
    return (static_cast<uint32_t>(bank & 0x7F) << 13)
        | static_cast<uint32_t>(address & 0x1FFF);
}

uint8_t GbMbc6::readFlash(uint32_t address) const
{
    address &= flashSize - 1;

    if (flashIdMode) {
        switch (address & 0x03) {
        case 0:
            return 0xC2;
        case 1:
            return 0x81;
        case 2:
            return address < flashSectorSize ? 0xC2 : 0x00;
        default:
            return 0xFF;
        }
    }

    if (flashHiddenRead) {
        return hiddenFlash.at(address & 0xFF);
    }

    if (flashStatusMode) {
        return static_cast<uint8_t>(0x80 | (sector0Protected ? 0x02 : 0x00));
    }

    return flash.at(address);
}

void GbMbc6::writeFlash(uint32_t address, uint8_t value)
{
    address &= flashSize - 1;

    if (value == 0xF0) {
        resetFlashCommand();
        return;
    }

    switch (flashPhase) {
    case FlashPhase::idle:
        if ((address & 0x7FFF) == 0x5555 && value == 0xAA) {
            flashPhase = FlashPhase::unlock1;
        }
        break;

    case FlashPhase::unlock1:
        if ((address & 0x7FFF) == 0x2AAA && value == 0x55) {
            flashPhase = FlashPhase::unlock2;
        } else {
            flashPhase = FlashPhase::idle;
        }
        break;

    case FlashPhase::unlock2:
        if ((address & 0x7FFF) != 0x5555) {
            flashPhase = FlashPhase::idle;
            break;
        }
        switch (value) {
        case 0x90:
            flashIdMode = true;
            flashStatusMode = false;
            flashHiddenRead = false;
            flashPhase = FlashPhase::idle;
            break;
        case 0xA0:
            startProgram();
            flashPhase = FlashPhase::program;
            break;
        case 0x80:
            flashPendingCommand = 0x80;
            flashPhase = FlashPhase::unlock3;
            break;
        case 0x60:
            flashPendingCommand = 0x60;
            flashPhase = FlashPhase::unlock3;
            break;
        case 0x77:
            flashPendingCommand = 0x77;
            flashPhase = FlashPhase::unlock3;
            break;
        default:
            flashPhase = FlashPhase::idle;
            break;
        }
        break;

    case FlashPhase::unlock3:
        if ((address & 0x7FFF) == 0x5555 && value == 0xAA) {
            flashPhase = FlashPhase::unlock4;
        } else {
            flashPhase = FlashPhase::idle;
        }
        break;

    case FlashPhase::unlock4:
        if ((address & 0x7FFF) == 0x2AAA && value == 0x55) {
            flashPhase = FlashPhase::unlock5;
        } else {
            flashPhase = FlashPhase::idle;
        }
        break;

    case FlashPhase::unlock5:
        handleFlashCommand(address, value);
        break;

    case FlashPhase::program:
        writeProgramBuffer(address, value, false);
        break;

    case FlashPhase::programHidden:
        writeProgramBuffer(address, value, true);
        break;
    }
}

void GbMbc6::handleFlashCommand(uint32_t address, uint8_t value)
{
    const uint8_t pendingCommand = flashPendingCommand;
    flashPendingCommand = 0;
    flashPhase = FlashPhase::idle;

    if (pendingCommand == 0x80) {
        if (value == 0x30) {
            eraseSector(address);
            flashStatusMode = true;
        } else if (value == 0x10) {
            eraseChip();
            flashStatusMode = true;
        }
        return;
    }

    if (pendingCommand == 0x60) {
        if (!flashWriteEnabled) {
            return;
        }
        if (value == 0x04) {
            eraseHidden();
            flashStatusMode = true;
        } else if (value == 0xE0) {
            startProgram();
            flashPhase = FlashPhase::programHidden;
        } else if (value == 0x40 || value == 0x20) {
            if (address >= flashSectorSize) {
                return;
            }
            sector0Protected = value == 0x20;
            flashStatusMode = true;
        }
        return;
    }

    if (pendingCommand == 0x77 && value == 0x77) {
        flashHiddenRead = true;
        flashIdMode = false;
        flashStatusMode = false;
    }
}

void GbMbc6::startProgram()
{
    programBuffer.fill(0xFF);
    programBufferValid = false;
    lastProgramOffset = 0;
    flashStatusMode = true;
    flashIdMode = false;
    flashHiddenRead = false;
}

void GbMbc6::writeProgramBuffer(uint32_t address, uint8_t value, bool hidden)
{
    const uint8_t offset = static_cast<uint8_t>(address & 0x7F);

    if (programBufferValid && offset == lastProgramOffset) {
        const uint32_t pageAddress = hidden
            ? (address & 0x80)
            : (address & ~static_cast<uint32_t>(0x7F));
        programPage(pageAddress, hidden);
        flashPhase = FlashPhase::idle;
        programBufferValid = false;
        return;
    }

    programBuffer.at(offset) = value;
    lastProgramOffset = offset;
    programBufferValid = true;
}

void GbMbc6::programPage(uint32_t address, bool hidden)
{
    if (hidden) {
        if (!flashWriteEnabled) {
            return;
        }
        for (std::size_t i = 0; i < flashPageSize; ++i) {
            const std::size_t index = (static_cast<std::size_t>(address) + i) & 0xFF;
            hiddenFlash.at(index) &= programBuffer.at(i);
        }
        return;
    }

    if (address < flashSectorSize && (!flashWriteEnabled || sector0Protected)) {
        return;
    }

    for (std::size_t i = 0; i < flashPageSize; ++i) {
        flash.at(static_cast<std::size_t>(address) + i) &= programBuffer.at(i);
    }
}

void GbMbc6::eraseSector(uint32_t address)
{
    const uint32_t sector = address / flashSectorSize;
    if (sector == 0 && (!flashWriteEnabled || sector0Protected)) {
        return;
    }

    const auto begin = flash.begin()
        + static_cast<std::ptrdiff_t>(sector * flashSectorSize);
    std::fill_n(begin, flashSectorSize, 0xFF);
}

void GbMbc6::eraseChip()
{
    const std::size_t firstSector = (flashWriteEnabled && !sector0Protected) ? 0 : 1;
    const auto begin = flash.begin()
        + static_cast<std::ptrdiff_t>(firstSector * flashSectorSize);
    std::fill(begin, flash.end(), 0xFF);
}

void GbMbc6::eraseHidden()
{
    if (!flashWriteEnabled) {
        return;
    }
    hiddenFlash.fill(0xFF);
}

void GbMbc6::resetFlashCommand()
{
    flashPhase = FlashPhase::idle;
    flashPendingCommand = 0;
    flashIdMode = false;
    flashStatusMode = false;
    flashHiddenRead = false;
    programBuffer.fill(0xFF);
    lastProgramOffset = 0;
    programBufferValid = false;
}

}  // namespace qnemu

/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/cartridge/mbc/GbMbc7.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>

namespace qnemu
{

GbMbc7::GbMbc7(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& /*ramBanks*/, uint8_t type,
        std::shared_ptr<AccelerometerState> accelerometer) :
    romBanks(std::move(romBanks)),
    type(type),
    accelerometer(accelerometer ? std::move(accelerometer)
                                : std::make_shared<AccelerometerState>())
{
    eeprom.fill(0xFF);
    GbMbc7::reset();
}

uint8_t GbMbc7::read(uint16_t address) const
{
    if (address <= MemoryRomBank01End) {
        if (romBanks.empty()) {
            return 0xFF;
        }
        if (address <= MemoryRomBank00End) {
            return romBanks.at(0).at(address);
        }
        const std::size_t bank = romBankNumber_ & (romBanks.size() - 1);
        return romBanks.at(bank).at(address - MemoryRomBank01Start);
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!registersEnabled() || address >= 0xB000) {
            return 0xFF;
        }
        return readRegister(static_cast<uint8_t>((address >> 4) & 0x0F));
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbMbc7::write(uint16_t address, const uint8_t value)
{
    if (address < romBankAddress) {
        ramEnable1_ = (value & 0x0F) == 0x0A;
        return;
    }
    if (address < ramEnable2Address) {
        romBankNumber_ = static_cast<uint8_t>(value & 0x7F);
        return;
    }
    if (address <= 0x5FFF) {
        ramEnable2_ = (value & 0x40) != 0;
        return;
    }
    if (address <= MemoryRomBank01End) {
        return;
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (registersEnabled() && address < 0xB000) {
            writeRegister(static_cast<uint8_t>((address >> 4) & 0x0F), value);
        }
        return;
    }

    assert(false && "Wrong address");
}

void GbMbc7::reset()
{
    ramEnable1_ = false;
    ramEnable2_ = false;
    romBankNumber_ = 1;
    accelerometerX_ = 0x8000;
    accelerometerY_ = 0x8000;
    accelerometerErased_ = false;
    eepromWriteEnabled_ = false;
    resetEepromCommand();
    eepromDo_ = true;
    eepromClock_ = false;
    eepromCs_ = false;
    eepromDi_ = false;
}

bool GbMbc7::hasBattery() const
{
    return type == 0x22;
}

std::size_t GbMbc7::ramSize() const
{
    return eeprom.size();
}

std::vector<uint8_t> GbMbc7::dumpRam() const
{
    return {eeprom.begin(), eeprom.end()};
}

void GbMbc7::loadRam(const std::vector<uint8_t>& data)
{
    for (std::size_t i = 0; i < eeprom.size(); ++i) {
        eeprom.at(i) = i < data.size() ? data.at(i) : 0xFF;
    }
}

uint8_t GbMbc7::readRegister(uint8_t reg) const
{
    switch (reg) {
    case 0x0:
    case 0x1:
        return 0xFF;
    case 0x2:
        return static_cast<uint8_t>(accelerometerX_ & 0xFF);
    case 0x3:
        return static_cast<uint8_t>(accelerometerX_ >> 8);
    case 0x4:
        return static_cast<uint8_t>(accelerometerY_ & 0xFF);
    case 0x5:
        return static_cast<uint8_t>(accelerometerY_ >> 8);
    case 0x6:
        return 0x00;
    case 0x7:
        return 0xFF;
    case 0x8:
        return static_cast<uint8_t>((eepromDo_ ? 0x01 : 0x00)
            | (eepromDi_ ? 0x02 : 0x00)
            | (eepromClock_ ? 0x40 : 0x00)
            | (eepromCs_ ? 0x80 : 0x00));
    default:
        return 0xFF;
    }
}

void GbMbc7::writeRegister(uint8_t reg, uint8_t value)
{
    switch (reg) {
    case 0x0:
        if (value == 0x55) {
            accelerometerX_ = 0x8000;
            accelerometerY_ = 0x8000;
            accelerometerErased_ = true;
        }
        break;
    case 0x1:
        if (value == 0xAA) {
            latchAccelerometer();
        }
        break;
    case 0x8: {
        const bool cs = (value & 0x80) != 0;
        const bool clock = (value & 0x40) != 0;
        eepromDi_ = (value & 0x02) != 0;
        if (!cs) {
            resetEepromCommand();
            eepromDo_ = true;
        } else {
            if (!eepromCs_) {
                resetEepromCommand();
            }
            if (clock && !eepromClock_) {
                clockEeprom(eepromDi_);
            }
        }
        eepromCs_ = cs;
        eepromClock_ = clock;
        break;
    }
    default:
        break;
    }
}

void GbMbc7::latchAccelerometer()
{
    if (!accelerometerErased_) {
        return;
    }
    const float x = accelerometer->x();
    const float y = accelerometer->y();
    accelerometerX_ = static_cast<uint16_t>(std::lround(
        static_cast<double>(accelerometerCenter) + x * accelerometerRange));
    accelerometerY_ = static_cast<uint16_t>(std::lround(
        static_cast<double>(accelerometerCenter) + y * accelerometerRange));
    accelerometerErased_ = false;
}

void GbMbc7::resetEepromCommand()
{
    eepromMode_ = EepromMode::command;
    eepromShift_ = 0;
    eepromBitCount_ = 0;
    eepromAddress_ = 0;
    eepromData_ = 0;
}

void GbMbc7::clockEeprom(bool dataIn)
{
    switch (eepromMode_) {
    case EepromMode::command:
        if (eepromBitCount_ == 0) {
            if (!dataIn) {
                return;
            }
            eepromShift_ = 1;
            eepromBitCount_ = 1;
            return;
        }
        eepromShift_ = static_cast<uint16_t>((eepromShift_ << 1) | (dataIn ? 1 : 0));
        ++eepromBitCount_;
        if (eepromBitCount_ == 10) {
            decodeEepromCommand();
        }
        break;

    case EepromMode::read:
        eepromDo_ = (eepromData_ & 0x8000) != 0;
        eepromData_ = static_cast<uint16_t>(eepromData_ << 1);
        ++eepromBitCount_;
        if (eepromBitCount_ == 16) {
            resetEepromCommand();
        }
        break;

    case EepromMode::write:
    case EepromMode::writeAll:
        eepromShift_ = static_cast<uint16_t>((eepromShift_ << 1) | (dataIn ? 1 : 0));
        ++eepromBitCount_;
        if (eepromBitCount_ == 16) {
            if (eepromWriteEnabled_) {
                if (eepromMode_ == EepromMode::write) {
                    setEepromWord(eepromAddress_, eepromShift_);
                } else {
                    for (std::size_t i = 0; i < eeprom.size(); i += 2) {
                        setEepromWord(static_cast<uint8_t>(i / 2), eepromShift_);
                    }
                }
            }
            resetEepromCommand();
        }
        break;
    }
}

void GbMbc7::decodeEepromCommand()
{
    const uint16_t command = eepromShift_;
    resetEepromCommand();
    if ((command & 0x200) == 0) {
        return;
    }

    const uint8_t opcode = static_cast<uint8_t>((command >> 7) & 0x03);
    eepromAddress_ = static_cast<uint8_t>((command >> 1) & 0x3F);
    switch (opcode) {
    case 0x0: {
        const uint8_t subcode = static_cast<uint8_t>((command >> 5) & 0x03);
        if (subcode == 0x0) {
            eepromWriteEnabled_ = false;
        } else if (subcode == 0x1) {
            eepromMode_ = EepromMode::writeAll;
        } else if (subcode == 0x2) {
            if (eepromWriteEnabled_) {
                for (std::size_t i = 0; i < eeprom.size(); i += 2) {
                    setEepromWord(static_cast<uint8_t>(i / 2), 0xFFFF);
                }
            }
        } else {
            eepromWriteEnabled_ = true;
        }
        break;
    }
    case 0x1:
        eepromMode_ = EepromMode::write;
        break;
    case 0x2:
        eepromData_ = eepromWord(eepromAddress_);
        eepromMode_ = EepromMode::read;
        break;
    case 0x3:
        if (eepromWriteEnabled_) {
            setEepromWord(eepromAddress_, 0xFFFF);
        }
        break;
    }
}

uint16_t GbMbc7::eepromWord(uint8_t address) const
{
    const std::size_t offset = static_cast<std::size_t>(address & 0x3F) * 2;
    return static_cast<uint16_t>(eeprom.at(offset)
        | (static_cast<uint16_t>(eeprom.at(offset + 1)) << 8));
}

void GbMbc7::setEepromWord(uint8_t address, uint16_t value)
{
    const std::size_t offset = static_cast<std::size_t>(address & 0x3F) * 2;
    eeprom.at(offset) = static_cast<uint8_t>(value & 0xFF);
    eeprom.at(offset + 1) = static_cast<uint8_t>(value >> 8);
}

}  // namespace qnemu

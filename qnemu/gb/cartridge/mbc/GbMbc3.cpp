/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbMbc3.h"

namespace qnemu
{

GbMbc3::GbMbc3(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(std::move(romBanks)),
    ramBanks(std::move(ramBanks)),
    type(type)
{
    GbMbc3::reset();
}

uint8_t GbMbc3::read(uint16_t address) const
{
    if (address <= MemoryRomBank01End) {
        if (address <= MemoryRomBank00End) {
            return romBanks.at(0).at(address);
        }

        uint8_t bank = static_cast<uint8_t>(romBankNumber & romBankMask());
        if (bank == 0) {
            bank = 1;
        }
        bank = static_cast<uint8_t>(bank & (romBanks.size() - 1));
        return romBanks.at(bank).at(address - MemoryRomBank01Start);
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled) {
            return 0xFF;
        }

        if (ramBankOrRtc >= 0x08 && ramBankOrRtc <= 0x0C) {
            return latchedRtc.at(ramBankOrRtc - 0x08);
        }

        if (ramBanks.empty()) {
            return 0xFF;
        }
        const uint8_t bank = static_cast<uint8_t>(ramBankOrRtc & (ramBanks.size() - 1));
        return ramBanks.at(bank).at(address - externalRamStart);
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbMbc3::write(uint16_t address, const uint8_t value)
{
    if (address < romBankNumberAddress) {
        ramEnabled = (value & 0x0F) == 0x0A;
        return;
    }
    if (address < ramBankOrRtcAddress) {
        romBankNumber = static_cast<uint8_t>(value & romBankMask());
        if (romBankNumber == 0) {
            romBankNumber = 1;
        }
        return;
    }
    if (address < latchClockAddress) {
        ramBankOrRtc = static_cast<uint8_t>(value & 0x0F);
        return;
    }
    if (address <= MemoryRomBank01End) {
        if (value == 0x00) {
            latchSequence = 1;
        } else if (value == 0x01 && latchSequence == 1) {
            latchClock();
            latchSequence = 0;
        } else {
            latchSequence = 0;
        }
        return;
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled) {
            return;
        }

        if (ramBankOrRtc >= 0x08 && ramBankOrRtc <= 0x0C) {
            const std::size_t index = ramBankOrRtc - 0x08;
            switch (index) {
            case 0:
                rtcSeconds = static_cast<uint8_t>(value & 0x3F);
                break;
            case 1:
                rtcMinutes = static_cast<uint8_t>(value & 0x3F);
                break;
            case 2:
                rtcHours = static_cast<uint8_t>(value & 0x1F);
                break;
            case 3:
                rtcDays = static_cast<uint16_t>((rtcDays & 0x100) | value);
                break;
            default:
                rtcDays = static_cast<uint16_t>((rtcDays & 0xFF) | ((value & 0x01) << 8));
                rtcHalted = (value & 0x40) != 0;
                rtcCarry = (value & 0x80) != 0;
                break;
            }
            updateRtcRegisterBytes();
            latchedRtc = rtc;
            return;
        }

        if (ramBanks.empty()) {
            return;
        }
        const uint8_t bank = static_cast<uint8_t>(ramBankOrRtc & (ramBanks.size() - 1));
        ramBanks.at(bank).at(address - externalRamStart) = value;
        return;
    }

    assert(false && "Wrong address");
}

void GbMbc3::step()
{
    if (type != 0x0F && type != 0x10) {
        return;
    }
    if (rtcHalted) {
        return;
    }

    ++rtcCycleAccumulator;
    if (rtcCycleAccumulator >= CpuFrequency) {
        rtcCycleAccumulator -= CpuFrequency;
        advanceSecond();
    }
}

void GbMbc3::reset()
{
    ramEnabled = false;
    romBankNumber = 1;
    ramBankOrRtc = 0;
    latchSequence = 0;

    rtcCycleAccumulator = 0;
    rtcSeconds = 0;
    rtcMinutes = 0;
    rtcHours = 0;
    rtcDays = 0;
    rtcHalted = false;
    rtcCarry = false;
    updateRtcRegisterBytes();
    latchedRtc = rtc;
}

bool GbMbc3::hasBattery() const
{
    return type == 0x0F || type == 0x10 || type == 0x13;
}

std::size_t GbMbc3::ramSize() const
{
    return ramBanks.size() * RamBankSize;
}

std::vector<uint8_t> GbMbc3::dumpRam() const
{
    std::vector<uint8_t> data;
    data.reserve(ramSize());
    for (const auto& bank : ramBanks) {
        data.insert(data.end(), bank.begin(), bank.end());
    }
    return data;
}

void GbMbc3::loadRam(const std::vector<uint8_t>& data)
{
    std::size_t offset = 0;
    for (auto& bank : ramBanks) {
        for (auto& byte : bank) {
            byte = offset < data.size() ? data.at(offset++) : 0xFF;
        }
    }
}

bool GbMbc3::hasRtc() const
{
    return type == 0x0F || type == 0x10;
}

std::vector<uint8_t> GbMbc3::dumpRtc()
{
    updateRtcRegisterBytes();
    return {rtc.begin(), rtc.end()};
}

void GbMbc3::loadRtc(const std::vector<uint8_t>& data)
{
    if (data.size() < 5) {
        return;
    }

    rtcSeconds = static_cast<uint8_t>(data.at(0) & 0x3F);
    rtcMinutes = static_cast<uint8_t>(data.at(1) & 0x3F);
    rtcHours = static_cast<uint8_t>(data.at(2) & 0x1F);
    rtcDays = static_cast<uint16_t>(((data.at(4) & 0x01) << 8) | data.at(3));
    rtcHalted = (data.at(4) & 0x40) != 0;
    rtcCarry = (data.at(4) & 0x80) != 0;
    updateRtcRegisterBytes();
    latchedRtc = rtc;
}

void GbMbc3::advanceRtc(uint64_t seconds)
{
    if (!hasRtc() || rtcHalted) {
        return;
    }

    uint64_t total = rtcSeconds + seconds;
    rtcSeconds = static_cast<uint8_t>(total % 60);
    total /= 60;

    total += rtcMinutes;
    rtcMinutes = static_cast<uint8_t>(total % 60);
    total /= 60;

    total += rtcHours;
    rtcHours = static_cast<uint8_t>(total % 24);
    total /= 24;

    total += rtcDays;
    if (total > 0x1FF) {
        rtcCarry = true;
        total %= 0x200;
    }
    rtcDays = static_cast<uint16_t>(total);
    updateRtcRegisterBytes();
}

void GbMbc3::latchClock()
{
    updateRtcRegisterBytes();
    latchedRtc = rtc;
}

void GbMbc3::updateRtcRegisterBytes()
{
    rtc.at(0) = rtcSeconds;
    rtc.at(1) = rtcMinutes;
    rtc.at(2) = rtcHours;
    rtc.at(3) = static_cast<uint8_t>(rtcDays & 0xFF);
    rtc.at(4) = static_cast<uint8_t>(((rtcDays >> 8) & 0x01)
        | (rtcHalted ? 0x40 : 0x00)
        | (rtcCarry ? 0x80 : 0x00));
}

void GbMbc3::advanceSecond()
{
    ++rtcSeconds;
    if (rtcSeconds < 60) {
        return;
    }
    rtcSeconds = 0;
    ++rtcMinutes;
    if (rtcMinutes < 60) {
        return;
    }
    rtcMinutes = 0;
    ++rtcHours;
    if (rtcHours < 24) {
        return;
    }
    rtcHours = 0;
    ++rtcDays;
    if (rtcDays > 0x1FF) {
        rtcDays = 0;
        rtcCarry = true;
    }
}

}  // namespace qnemu

/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbAccelerometerState.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

/**
 * MBC7: ROM banking, a 256-byte 93LC56 EEPROM and a latched two-axis
 * accelerometer.  Used by Kirby Tilt 'n' Tumble.
 */
class GbMbc7 : public GbMbcInterface
{
public:
    static constexpr uint16_t ramEnableAddress = 0x0000;
    static constexpr uint16_t romBankAddress = 0x2000;
    static constexpr uint16_t ramEnable2Address = 0x4000;
    static constexpr uint16_t externalRamStart = 0xA000;
    static constexpr uint16_t externalRamEnd = 0xBFFF;

    static constexpr std::size_t eepromSize = 0x100;
    static constexpr uint16_t accelerometerCenter = 0x81D0;
    static constexpr int accelerometerRange = 0x70;

    GbMbc7() = delete;
    GbMbc7(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type,
        std::shared_ptr<AccelerometerState> accelerometer = nullptr);
    ~GbMbc7() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t value) override;
    void reset() override;

    bool hasBattery() const override;
    std::size_t ramSize() const override;
    std::vector<uint8_t> dumpRam() const override;
    void loadRam(const std::vector<uint8_t>& data) override;

private:
    enum class EepromMode
    {
        command,
        read,
        write,
        writeAll,
    };

    bool registersEnabled() const { return ramEnable1_ && ramEnable2_; }
    uint8_t readRegister(uint8_t reg) const;
    void writeRegister(uint8_t reg, uint8_t value);
    void latchAccelerometer();

    void resetEepromCommand();
    void clockEeprom(bool dataIn);
    void decodeEepromCommand();
    uint16_t eepromWord(uint8_t address) const;
    void setEepromWord(uint8_t address, uint16_t value);

    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    uint8_t type;
    std::shared_ptr<AccelerometerState> accelerometer;

    bool ramEnable1_ = false;
    bool ramEnable2_ = false;
    uint8_t romBankNumber_ = 1;

    uint16_t accelerometerX_ = 0x8000;
    uint16_t accelerometerY_ = 0x8000;
    bool accelerometerErased_ = false;

    std::array<uint8_t, eepromSize> eeprom {};
    bool eepromWriteEnabled_ = false;
    EepromMode eepromMode_ = EepromMode::command;
    uint16_t eepromShift_ = 0;
    uint8_t eepromBitCount_ = 0;
    uint8_t eepromAddress_ = 0;
    uint16_t eepromData_ = 0;
    bool eepromDo_ = true;
    bool eepromClock_ = false;
    bool eepromCs_ = false;
    bool eepromDi_ = false;
};

}  // namespace qnemu

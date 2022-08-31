/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

class GbVideoRam : public GbDeviceInterface
{
public:
    GbVideoRam() = delete;
    explicit GbVideoRam(const GbCartridgeInterface& cartridge);
    ~GbVideoRam() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    const std::array<uint8_t, VideoRamBankSize>& getBank(uint8_t index) const;

private:
    struct {
        uint8_t videoRamBank;  // FF4F
        uint8_t newDMASourceHigh;  // FF51
        uint8_t newDMASourceLow;  // FF52
        uint8_t newDMADestinationHigh;  // FF53
        uint8_t newDMADestinationLow;  // FF54
        uint8_t newDMALength;  // FF55
    } registers;
    const GbCartridgeInterface& cartridge;
    std::array<std::array<uint8_t, VideoRamBankSize>, 2> videoRamBanks;
};

}  // namespace qnemu

/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemu
{

class GbVideoRam : public GbDeviceInterface
{
public:
    GbVideoRam() = delete;
    explicit GbVideoRam(const GbCartridgeInterface& cartridge, const GbWorkRam& workRam);
    ~GbVideoRam() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    const std::array<uint8_t, VideoRamBankSize>& getBank(uint8_t index) const;
    void setModeFlag(uint8_t modeFlag);

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
    const GbWorkRam& workRam;
    uint16_t destination;
    uint16_t dmaTicks;
    bool isHBlankDma;
    bool isDmaInProgress;
    uint16_t length;
    uint8_t modeFlag;
    uint16_t source;
    std::array<std::array<uint8_t, VideoRamBankSize>, 2> videoRamBanks;
};

}  // namespace qnemu

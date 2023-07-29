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

class GbOam : public GbDeviceInterface
{
public:
    GbOam() = delete;
    explicit GbOam(const GbCartridgeInterface& cartridge,
        const GbDeviceInterface& videoRam,
        const GbDeviceInterface& workRam);
    ~GbOam() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t at(uint16_t address) const;

private:
    struct {
        uint8_t dmaTransferAndStartAddress;  // FF46
    } registers;

    std::array<uint8_t, OamSize> data;
    uint16_t dmaTicks;
    const GbCartridgeInterface& cartridge;
    const GbDeviceInterface& videoRam;
    const GbDeviceInterface& workRam;

    bool isDmaInProgress;
};

}  // namespace qnemu
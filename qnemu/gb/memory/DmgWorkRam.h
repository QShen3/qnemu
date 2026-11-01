/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

/**
 * DMG work RAM.
 *
 * C000-CFFF is the fixed lower 4 KiB, D000-DFFF is the fixed upper 4 KiB,
 * and E000-FDFF mirrors them.  FF70/SVBK does not exist on DMG hardware, so
 * reads return $FF and writes are ignored.
 */
class DmgWorkRam : public GbDeviceInterface
{
public:
    DmgWorkRam();
    ~DmgWorkRam() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

protected:
    std::array<std::array<uint8_t, 0x1000>, 8> data_{};
};

}  // namespace qnemu

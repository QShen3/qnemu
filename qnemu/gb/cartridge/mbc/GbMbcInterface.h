/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "qnemu/save/SaveTarget.h"

namespace qnemu
{

class GbMbcInterface : public SaveTarget
{
public:
    ~GbMbcInterface() override = default;

    virtual uint8_t read(uint16_t address) const = 0;
    virtual void write(uint16_t address, const uint8_t value) = 0;
    virtual void reset() = 0;

    /// Per-T-cycle hook; only the MBC3 RTC currently needs it.
    virtual void step() {}

    bool hasBattery() const override { return false; }
    std::size_t ramSize() const override { return 0; }
    std::vector<uint8_t> dumpRam() const override { return {}; }
    void loadRam(const std::vector<uint8_t>&) override {}

    bool hasRtc() const override { return false; }
    std::vector<uint8_t> dumpRtc() override { return {}; }
    void loadRtc(const std::vector<uint8_t>&) override {}
    void advanceRtc(uint64_t) override {}
};

}  // namespace qnemu
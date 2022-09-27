/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"

namespace qnemu
{

class SpriteAttributeTable : public GbDeviceInterface
{
public:
    SpriteAttributeTable() = delete;
    explicit SpriteAttributeTable(std::shared_ptr<GbCpuInterface> cpu, const GbGpuInterface& gpu);
    ~SpriteAttributeTable() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t at(uint16_t address) const;

private:
    struct {
        uint8_t dmaTransferAndStartAddress;  // FF46
    } registers;

    std::weak_ptr<GbCpuInterface> cpu;
    std::array<uint8_t, SpriteAttributeTableSize> data;
    uint16_t dmaTicks;
    const GbGpuInterface& gpu;
    bool isDmaInProgress;
};

}  // namespace qnemu
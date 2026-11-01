/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/gb/infrared/GbInfraredLink.h"

namespace qnemu
{

/**
 * Game Boy Color infrared port (FF56 RP).
 *
 * Bit 0 drives the LED, bit 1 reports the peer's signal, bits 6-7 enable
 * reading, and unused bits read as 1.  The sensor adapts to a continuous
 * signal after roughly 3 ms (Pan Docs "signal fade").
 */
class GbInfrared
{
public:
    explicit GbInfrared(std::shared_ptr<GbInfraredLink> link);
    ~GbInfrared();

    GbInfrared(const GbInfrared&) = delete;
    GbInfrared& operator=(const GbInfrared&) = delete;

    uint8_t read(uint16_t address) const;
    void write(uint16_t address, const uint8_t& value);
    void step();
    void reset();

private:
    bool signalDetected() const;

    std::shared_ptr<GbInfraredLink> link;
    uint8_t control = 0;
    uint32_t signalCycles = 0;
    bool faded = false;
};

}  // namespace qnemu

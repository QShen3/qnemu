/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace qnemu
{

/**
 * Cartridge persistence interface used by SaveManager.
 *
 * The emulator never interprets the game's save data: it only dumps or loads
 * the raw cartridge RAM (and, for MBC3, the RTC hardware state).
 */
class SaveTarget
{
public:
    virtual ~SaveTarget() = default;

    virtual bool hasBattery() const { return false; }
    virtual std::size_t ramSize() const { return 0; }
    virtual std::vector<uint8_t> dumpRam() const { return {}; }
    virtual void loadRam(const std::vector<uint8_t>& /*data*/) {}

    virtual bool hasRtc() const { return false; }
    virtual std::vector<uint8_t> dumpRtc() { return {}; }
    virtual void loadRtc(const std::vector<uint8_t>& /*data*/) {}
    virtual void advanceRtc(uint64_t /*seconds*/) {}
};

}  // namespace qnemu

/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/mmu/CgbMmu.h"

#include <cstdint>
#include <memory>
#include <utility>

namespace qnemu
{

CgbMmu::CgbMmu(std::unique_ptr<GbApu> apu,
        std::shared_ptr<GbCartridgeInterface> cartridge,
        std::unique_ptr<GbGpuInterface> gpu,
        std::unique_ptr<GbHighRam> highRam,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::unique_ptr<GbJoypad> joypad,
        std::unique_ptr<CgbWorkRam> workRam,
        std::unique_ptr<GbTimer> timer,
        std::unique_ptr<GbSerial> serial) :
    DmgMmu(std::move(apu), cartridge, std::move(gpu), std::move(highRam),
        interruptHandler, std::move(joypad), std::move(workRam), std::move(timer),
        std::move(serial))
{
}

uint8_t CgbMmu::read(uint16_t address) const
{
    if (address == 0xFF4F || (address >= 0xFF51 && address <= 0xFF55)
        || (address >= 0xFF68 && address <= 0xFF6C)) {
        return gpu().read(address);
    }
    if (address == 0xFF70) {
        return cgbWorkRam().read(address);
    }
    if (address >= 0xFF72 && address <= 0xFF75) {
        if (address == 0xFF74 && dmgMode_) {
            return 0xFF;
        }
        return address == 0xFF75
            ? static_cast<uint8_t>(0x8F | (cgbUnmappedRegisters_.at(3) & 0x70))
            : cgbUnmappedRegisters_.at(address - 0xFF72);
    }
    return DmgMmu::read(address);
}

void CgbMmu::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF4F || (address >= 0xFF51 && address <= 0xFF55)
        || (address >= 0xFF68 && address <= 0xFF6C)) {
        gpu().write(address, value);
        return;
    }
    if (address == 0xFF70) {
        cgbWorkRam().write(address, value);
        return;
    }
    if (address >= 0xFF72 && address <= 0xFF75) {
        cgbUnmappedRegisters_.at(address - 0xFF72) =
            address == 0xFF75 ? static_cast<uint8_t>(value & 0x70) : value;
        return;
    }
    DmgMmu::write(address, value);
}

void CgbMmu::reset()
{
    DmgMmu::reset();
    cgbUnmappedRegisters_.fill(0);
    dmgMode_ = false;
    cgbWorkRam().setDmgCompatibility(false);
}

void CgbMmu::setCgb(bool cgb)
{
    serial().setCgbMode(cgb);
    if (cgb) {
        dmgMode_ = false;
        cgbWorkRam().setDmgCompatibility(false);
    } else {
        setDmgCompatibilityMode(true);
    }
}

void CgbMmu::setDmgCompatibilityMode(bool enabled)
{
    dmgMode_ = enabled;
    serial().setCgbMode(!enabled);
    cgbWorkRam().setDmgCompatibility(enabled);
}

void CgbMmu::setDoubleSpeed(bool doubleSpeed)
{
    timer().setDoubleSpeed(doubleSpeed);
    serial().setDoubleSpeed(doubleSpeed);
}

CgbWorkRam& CgbMmu::cgbWorkRam() const
{
    return static_cast<CgbWorkRam&>(workRam());
}

}  // namespace qnemu

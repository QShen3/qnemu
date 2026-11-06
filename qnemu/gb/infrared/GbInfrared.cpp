/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/infrared/GbInfrared.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

namespace qnemu
{

namespace
{

// Pan Docs: the sensor fades after about 3 ms of continuous IR light.
constexpr uint32_t fadeCycles = 12583;

}  // namespace

GbInfrared::GbInfrared(std::shared_ptr<GbInfraredLink> infraredLink) :
    link(std::move(infraredLink))
{
    reset();
}

GbInfrared::~GbInfrared()
{
    if (link) {
        link->reset();
    }
}

uint8_t GbInfrared::read(uint16_t address) const
{
    assert(address == 0xFF56);
    (void)address;
    uint8_t value = static_cast<uint8_t>((control & 0xC1) | 0x3E);
    if (signalDetected()) {
        value = static_cast<uint8_t>(value & static_cast<uint8_t>(~0x02));
    }
    return value;
}

void GbInfrared::write(uint16_t address, const uint8_t& value)
{
    assert(address == 0xFF56);
    (void)address;
    control = static_cast<uint8_t>(value & 0xC1);
    if (link) {
        link->setLed((control & 0x01) != 0);
    }
    if ((control & 0xC0) != 0xC0) {
        signalCycles = 0;
        faded = false;
    }
}

void GbInfrared::step()
{
    if ((control & 0xC0) != 0xC0 || !link) {
        signalCycles = 0;
        faded = false;
        return;
    }

    if (link->peerLed()) {
        if (signalCycles < fadeCycles) {
            ++signalCycles;
        }
        if (signalCycles >= fadeCycles) {
            faded = true;
        }
    } else {
        signalCycles = 0;
        faded = false;
    }
}

void GbInfrared::reset()
{
    control = 0;
    signalCycles = 0;
    faded = false;
    if (link) {
        link->setLed(false);
        link->reset();
    }
}

bool GbInfrared::signalDetected() const
{
    return (control & 0xC0) == 0xC0 && !faded && link && link->peerLed();
}

}  // namespace qnemu

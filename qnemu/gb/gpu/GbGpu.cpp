/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <tuple>
#include <utility>

#include <QtGui/QColor>
#include <QtGui/QImage>

#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/gpu/GbPalette.h"
#include "qnemu/gb/gpu/GbVideoRam.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/GbRender.h"
#include "qnemu/gb/gpu/Mode.h"
#include "qnemu/gb/gpu/GbOam.h"

namespace qnemu
{

GbGpu::GbGpu(std::unique_ptr<GbPalette> palette,
        std::unique_ptr<GbcPalette> coloredPalette,
        std::unique_ptr<GbOam> oam,
        std::unique_ptr<GbVideoRam> videoRam,
        std::unique_ptr<GbRender> render) :
    palette(std::move(palette)),
    coloredPalette(std::move(coloredPalette)),
    oam(std::move(oam)),
    videoRam(std::move(videoRam)),
    render(std::move(render)),
    modes({
        Mode
        { "Mode0", 204, [this](){mode0();} },
        { "Mode1", 456, [this](){mode1();} },
        { "Mode2", 80,  [this](){mode2();} },
        { "Mode3", 172, [this](){mode3();} },
    })
{
    GbGpu::reset();
}

GbGpu::~GbGpu()
{
}

uint8_t GbGpu::read(uint16_t address) const
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        if (currentMode() == 3 && isLcdEnable()) {
            return 0xFF;
        }
        return videoRam->read(address);
    } else if (address >= OamStart && address <= OamEnd) {
        if ((currentMode() == 2 || currentMode() == 3) && isLcdEnable()) {
            return 0xFF;
        }
        return oam->read(address);
    } else if (address >= 0xFF40 && address <= 0xFF45) {
        return render->read(address);
    } else if (address == 0xFF46) {
        return oam->read(address);
    } else if (address >= 0xFF47 && address <= 0xFF49) {
        return palette->read(address);
    } else if (address >= 0xFF4A && address <= 0xFF4B) {
        return render->read(address);
    } else if (address == 0xFF4F) {
        return videoRam->read(address);
    } else if (address >= 0xFF51 && address <= 0xFF55) {
        return videoRam->read(address);
    } else if (address >= 0xFF68 && address <= 0xFF6B) {
        if (address == 0xFF69 || address == 0xFF6B) {
            if (currentMode() == 3 && isLcdEnable()) {
                return 0xFF;
            }
        }
        return coloredPalette->read(address);
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbGpu::write(uint16_t address, const uint8_t& value)
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        if (currentMode() == 3 && isLcdEnable()) {
            return;
        }
        videoRam->write(address, value);
    } else if (address >= OamStart && address <= OamEnd) {
        if ((currentMode() == 2 || currentMode() == 3) && isLcdEnable()) {
            return;
        }
        oam->write(address, value);
    } else if (address >= 0xFF40 && address <= 0xFF45) {
        render->write(address, value);
    }
    else if (address == 0xFF46) {
        oam->write(address, value);
    } else if (address >= 0xFF47 && address <= 0xFF49) {
        palette->write(address, value);
    } else if (address >= 0xFF4A && address <= 0xFF4B) {
        render->write(address, value);
    }
    else if (address == 0xFF4F) {
        videoRam->write(address, value);
    } else if (address >= 0xFF51 && address <= 0xFF55) {
        videoRam->write(address, value);
    } else if (address >= 0xFF68 && address <= 0xFF6B) {
        if (address == 0xFF69 || address == 0xFF6B) {
            if (currentMode() == 3 && isLcdEnable() == 1) {
                return;
            }
        }
        coloredPalette->write(address, value);
    }
}

void GbGpu::step()
{
    palette->step();
    coloredPalette->step();
    oam->step();
    videoRam->step();
    render->step();
    if (!isLcdEnable()) {
        ticks = 0;
        return;
    }
    if (ticks == 0) {
        auto mode = modes.at(currentMode());
        ticks = mode.ticks;
        mode.execute();
        videoRam->setModeFlag(currentMode());
    } else {
        ticks--;
    }
}

void GbGpu::reset()
{
    palette->reset();
    coloredPalette->reset();
    oam->reset();
    videoRam->reset();
    render->reset();
}

uint8_t GbGpu::currentMode() const
{
    return render->currentMode();
}

bool GbGpu::isLcdEnable() const
{
    return render->isLcdEnable();
}

}  // namespace qnemu

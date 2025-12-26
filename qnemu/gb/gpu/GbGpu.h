/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <vector>

#include <QtGui/QColor>
#include <QtGui/QImage>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/gpu/GbPalette.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/gpu/GbVideoRam.h"
#include "qnemu/gb/gpu/GbRender.h"
#include "qnemu/gb/gpu/Mode.h"
#include "qnemu/gb/gpu/GbOam.h"

namespace qnemu
{

class GbGpu : public GbGpuInterface
{
public:
    GbGpu() = delete;
    GbGpu(std::unique_ptr<GbPalette> palette,
        std::unique_ptr<GbcPalette> coloredPalette,
        std::unique_ptr<GbOam> oam,
        std::unique_ptr<GbVideoRam> videoRam,
        std::unique_ptr<GbRender> gbRender);
    ~GbGpu() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t currentMode() const override;
    bool isLcdEnable() const override;

private:
    std::unique_ptr<GbPalette> palette;
    std::unique_ptr<GbcPalette> coloredPalette;
    std::unique_ptr<GbOam> oam;
    std::unique_ptr<GbVideoRam> videoRam;
    std::unique_ptr<GbRender> render;
    uint16_t ticks;
    const std::array<Mode, 4> modes;

    // Modes
    void mode0();
    void mode1();
    void mode2();
    void mode3();
};

}  // namespace qnemu
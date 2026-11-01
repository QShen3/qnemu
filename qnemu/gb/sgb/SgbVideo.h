/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <span>

#include "qnemu/gb/gpu/PpuColorizer.h"
#include "qnemu/gb/gpu/PpuFrameSink.h"

namespace qnemu
{

class DisplayInterface;
class FrameBuffer;
class GbSgb;

/// Applies the SGB attribute palette to a DMG shade.
class SgbColorizer final : public PpuColorizer
{
public:
    explicit SgbColorizer(GbSgb& sgb) : sgb_(sgb) {}

    uint32_t colorForDmgPixel(int x, int y,
        uint8_t colorIndex, uint8_t palette) const override;

private:
    GbSgb& sgb_;
};

/// Processes SGB transfers and presents the framed 256x224 image.
class SgbFrameSink final : public PpuFrameSink
{
public:
    SgbFrameSink(GbSgb& sgb, DisplayInterface& display) :
        sgb_(sgb), display_(display) {}

    void present(const FrameBuffer& frame,
        std::span<const uint8_t> vram) override;

private:
    GbSgb& sgb_;
    DisplayInterface& display_;
};

}  // namespace qnemu

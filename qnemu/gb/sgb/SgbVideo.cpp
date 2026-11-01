/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/sgb/SgbVideo.h"

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/display/FrameBuffer.h"
#include "qnemu/display/SgbFrameBuffer.h"
#include "qnemu/gb/sgb/GbSgb.h"

namespace qnemu
{

uint32_t SgbColorizer::colorForDmgPixel(int x, int y,
    uint8_t colorIndex, uint8_t palette) const
{
    const uint8_t shade = static_cast<uint8_t>((palette >> (colorIndex * 2)) & 3);
    return sgb_.colorForPixel(x, y, shade);
}

void SgbFrameSink::present(const FrameBuffer& frame, std::span<const uint8_t> vram)
{
    if (sgb_.transferPending()) {
        sgb_.processTransfers(vram.data(), vram.size());
    }
    SgbFrameBuffer sgbFrame;
    sgb_.renderBorder(frame, sgbFrame);
    display_.presentSgbFrame(sgbFrame);
}

}  // namespace qnemu

/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/gpu/DmgPpuVideo.h"

namespace qnemu
{

DmgPpuVideo::DmgPpuVideo() : fifo_(false), renderer_(false)
{
}

void DmgPpuVideo::reset()
{
}

void DmgPpuVideo::setColorizer(PpuColorizer* colorizer)
{
    renderer_.setColorizer(colorizer);
    fifo_.setColorizer(colorizer);
}

void DmgPpuVideo::startLine(PpuRegisters& registers, PpuMemory& memory,
    uint8_t line, uint16_t windowLine)
{
    fifo_.startLine(registers, memory, line, windowLine);
}

void DmgPpuVideo::step(PpuRegisters& registers, PpuMemory& memory, FrameBuffer& frame)
{
    fifo_.step(registers, memory, frame);
}

void DmgPpuVideo::renderSprites(PpuRegisters& registers, PpuMemory& memory,
    uint8_t line, FrameBuffer& frame)
{
    renderer_.renderSprites(registers, memory, line,
        fifo_.backgroundColorIndex(), fifo_.backgroundPriority(), frame);
}

}  // namespace qnemu

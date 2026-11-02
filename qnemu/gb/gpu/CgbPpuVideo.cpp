/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/gpu/CgbPpuVideo.h"

#include <utility>

#include "qnemu/gb/gpu/PpuMemory.h"
#include "qnemu/gb/gpu/PpuRegisters.h"

namespace qnemu
{

namespace
{
constexpr uint16_t vramBankAddress = 0xFF4F;
constexpr uint16_t hdma1Address = 0xFF51;
constexpr uint16_t hdma2Address = 0xFF52;
constexpr uint16_t hdma3Address = 0xFF53;
constexpr uint16_t hdma4Address = 0xFF54;
constexpr uint16_t hdma5Address = 0xFF55;
constexpr uint16_t bgPaletteSpecAddress = 0xFF68;
constexpr uint16_t bgPaletteDataAddress = 0xFF69;
constexpr uint16_t objPaletteSpecAddress = 0xFF6A;
constexpr uint16_t objPaletteDataAddress = 0xFF6B;
constexpr uint16_t objectPriorityAddress = 0xFF6C;
}  // namespace

CgbPpuVideo::CgbPpuVideo() : fifo_(true), renderer_(true)
{
}

void CgbPpuVideo::reset()
{
    dmgMode_ = false;
    hdmaActive_ = false;
    hdmaHblank_ = false;
    hdma5_ = 0xFF;
    hdmaRemaining_ = 0;
    hdmaTicks_ = 0;
    hdmaSource_ = 0x0000;
    hdmaDestination_ = 0x8000;
}

bool CgbPpuVideo::readRegister(uint16_t address, const PpuRegisters& registers,
    const PpuMemory& memory, uint8_t& value) const
{
    switch (address) {
        case vramBankAddress:
            value = static_cast<uint8_t>(0xFE | memory.vramBank());
            return true;
        case hdma1Address:
        case hdma2Address:
        case hdma3Address:
        case hdma4Address:
            // HDMA1-4 are write-only and read back as $FF.
            value = 0xFF;
            return true;
        case hdma5Address:
            value = hdma5_;
            return true;
        case bgPaletteSpecAddress:
            value = memory.readPaletteSpec(false);
            return true;
        case bgPaletteDataAddress:
            value = (registers.mode() == 3 && registers.lcdEnabled())
                ? 0xFF
                : memory.readPaletteData(false);
            return true;
        case objPaletteSpecAddress:
            value = memory.readPaletteSpec(true);
            return true;
        case objPaletteDataAddress:
            value = (registers.mode() == 3 && registers.lcdEnabled())
                ? 0xFF
                : memory.readPaletteData(true);
            return true;
        case objectPriorityAddress:
            value = static_cast<uint8_t>(0xFE | (registers.opri & 0x01));
            return true;
        default:
            return false;
    }
}

bool CgbPpuVideo::writeRegister(uint16_t address, uint8_t value,
    PpuRegisters& registers, PpuMemory& memory)
{
    switch (address) {
        case vramBankAddress:
            memory.setVramBank(value);
            return true;
        case hdma1Address:
            // Source can live in ROM/WRAM/echo RAM; keep the full high byte.
            // Masking it to 5 bits aliases 0xA000-0xDFFF to 0x0000-0x1FFF.
            hdmaSource_ = static_cast<uint16_t>((hdmaSource_ & 0x00F0)
                | (value << 8));
            return true;
        case hdma2Address:
            hdmaSource_ = static_cast<uint16_t>((hdmaSource_ & 0xFF00)
                | (value & 0xF0));
            return true;
        case hdma3Address:
            hdmaDestination_ = static_cast<uint16_t>(0x8000
                | ((value & 0x1F) << 8) | (hdmaDestination_ & 0x00F0));
            return true;
        case hdma4Address:
            hdmaDestination_ = static_cast<uint16_t>((hdmaDestination_ & 0xFF00)
                | (value & 0xF0));
            return true;
        case hdma5Address:
            writeHdma5(value, registers, memory);
            return true;
        case bgPaletteSpecAddress:
            memory.writePaletteSpec(false, value);
            return true;
        case bgPaletteDataAddress:
            if (!(registers.mode() == 3 && registers.lcdEnabled())) {
                memory.writePaletteData(false, value);
            }
            return true;
        case objPaletteSpecAddress:
            memory.writePaletteSpec(true, value);
            return true;
        case objPaletteDataAddress:
            if (!(registers.mode() == 3 && registers.lcdEnabled())) {
                memory.writePaletteData(true, value);
            }
            return true;
        case objectPriorityAddress:
            registers.opri = static_cast<uint8_t>(value & 0x01);
            return true;
        default:
            return false;
    }
}

void CgbPpuVideo::setExternalRead(std::function<uint8_t(uint16_t)> reader)
{
    externalRead_ = std::move(reader);
}

void CgbPpuVideo::setColorizer(PpuColorizer* colorizer)
{
    renderer_.setColorizer(colorizer);
    fifo_.setColorizer(colorizer);
}

void CgbPpuVideo::stepHdma(PpuMemory& memory)
{
    if (!hdmaActive_ || hdmaHblank_) {
        return;
    }
    ++hdmaTicks_;
    if (hdmaTicks_ >= 32) {
        hdmaTicks_ = 0;
        transferHdmaBlock(memory);
    }
}

void CgbPpuVideo::onMode0(PpuMemory& memory)
{
    if (hdmaActive_ && hdmaHblank_) {
        transferHdmaBlock(memory);
    }
}

void CgbPpuVideo::startLine(PpuRegisters& registers, PpuMemory& memory,
    uint8_t line, uint16_t windowLine)
{
    fifo_.startLine(registers, memory, line, windowLine);
}

void CgbPpuVideo::step(PpuRegisters& registers, PpuMemory& memory, FrameBuffer& frame)
{
    fifo_.step(registers, memory, frame);
}

void CgbPpuVideo::renderSprites(PpuRegisters& registers, PpuMemory& memory,
    uint8_t line, FrameBuffer& frame)
{
    renderer_.renderSprites(registers, memory, line,
        fifo_.backgroundColorIndex(), fifo_.backgroundPriority(), frame);
}

void CgbPpuVideo::setDmgMode(bool dmgMode)
{
    dmgMode_ = dmgMode;
    renderer_.setDmgMode(dmgMode);
    fifo_.setDmgMode(dmgMode);
}

void CgbPpuVideo::transferHdmaBlock(PpuMemory& memory)
{
    if (!hdmaActive_ || hdmaRemaining_ == 0) {
        return;
    }
    for (uint16_t index = 0; index < 0x10; ++index) {
        const uint16_t source = static_cast<uint16_t>(hdmaSource_ + index);
        const uint8_t data = externalRead_ ? externalRead_(source) : 0xFF;
        const uint16_t destination = static_cast<uint16_t>(
            0x8000 | ((hdmaDestination_ + index) & 0x1FFF));
        memory.writeVram(destination, data);
    }
    hdmaSource_ = static_cast<uint16_t>(hdmaSource_ + 0x10);
    hdmaDestination_ = static_cast<uint16_t>(0x8000 | ((hdmaDestination_ + 0x10) & 0x1FFF));
    --hdmaRemaining_;
    if (hdmaRemaining_ == 0) {
        hdmaActive_ = false;
        hdma5_ = 0xFF;
    } else {
        hdma5_ = static_cast<uint8_t>((hdma5_ & 0x80) | (hdmaRemaining_ - 1));
    }
}

void CgbPpuVideo::writeHdma5(uint8_t value, PpuRegisters& registers, PpuMemory& memory)
{
    if (hdmaActive_ && (value & 0x80) == 0) {
        hdmaActive_ = false;
        hdma5_ = 0xFF;
        return;
    }
    if (hdmaActive_) {
        return;
    }
    hdmaRemaining_ = static_cast<uint8_t>((value & 0x7F) + 1);
    hdmaHblank_ = (value & 0x80) != 0;
    hdma5_ = static_cast<uint8_t>(value & 0x7F);
    hdmaActive_ = true;
    hdmaTicks_ = 0;
    if (hdmaHblank_ && registers.lcdEnabled() && registers.mode() == 0) {
        // An HBlank DMA started during HBlank moves its first block right
        // away, before the CPU can change VBK for the following block.
        transferHdmaBlock(memory);
    }
}

}  // namespace qnemu

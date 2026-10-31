/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <utility>

#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbPpu.h"

namespace qnemu
{

namespace
{
constexpr uint16_t oamDmaAddress = 0xFF46;
constexpr uint16_t lcdcAddress = 0xFF40;
constexpr uint16_t statAddress = 0xFF41;
constexpr uint16_t scyAddress = 0xFF42;
constexpr uint16_t scxAddress = 0xFF43;
constexpr uint16_t lyAddress = 0xFF44;
constexpr uint16_t lycAddress = 0xFF45;
constexpr uint16_t bgpAddress = 0xFF47;
constexpr uint16_t obp0Address = 0xFF48;
constexpr uint16_t obp1Address = 0xFF49;
constexpr uint16_t wyAddress = 0xFF4A;
constexpr uint16_t wxAddress = 0xFF4B;
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

namespace
{

uint8_t bitwiseGlitch(uint8_t a, uint8_t b, uint8_t c)
{
    return static_cast<uint8_t>(((a ^ c) & (b ^ c)) ^ c);
}

uint16_t bitwiseGlitchReadWord(uint16_t a, uint16_t b, uint16_t c)
{
    return static_cast<uint16_t>(b | (a & c));
}

uint16_t bitwiseGlitchTertiary1(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e)
{
    return static_cast<uint16_t>(c | (a & b & d & e));
}

uint16_t bitwiseGlitchTertiary2(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e)
{
    return static_cast<uint16_t>((c & (a | b | d | e)) | (a & b & d & e));
}

uint16_t bitwiseGlitchTertiary3(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e)
{
    return static_cast<uint16_t>((c & (a | b | d | e)) | (b & d & e));
}

uint16_t bitwiseGlitchQuaternaryDmg(uint16_t /*a*/, uint16_t b, uint16_t c, uint16_t d,
    uint16_t e, uint16_t f, uint16_t g, uint16_t h)
{
    return static_cast<uint16_t>((e & (h | g | (static_cast<uint16_t>(~d) & f) | c | b)) | (c & g & h));
}

uint8_t bitwiseGlitchReadIncrease(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return static_cast<uint8_t>((b & a) | (b & c) | (b & d) | (a & c & d));
}

}  // namespace

GbPpu::GbPpu(bool cgb,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler) :
    cgb_(cgb),
    display_(std::move(display)),
    interruptHandler_(std::move(interruptHandler)),
    renderer_(cgb),
    fifo_(cgb)
{
    GbPpu::reset();
}

uint8_t GbPpu::read(uint16_t address) const
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        if (vramReadBlocked()) {
            return 0xFF;
        }
        return memory_.readVram(address);
    }

    if (address >= OamStart && address < 0xFF00) {
        if (oamReadBlocked()) {
            if (oamBugEnabled_) {
                // The read itself is what triggers the DMG OAM corruption;
                // the method has to mutate OAM even though the bus read is
                // logically const.
                const_cast<GbPpu*>(this)->triggerOamBugRead(address);
            }
            return 0xFF;
        }
        if (address <= OamEnd) {
            return memory_.readOam(address);
        }
        return 0xFF;
    }

    switch (address) {
        case lcdcAddress: return registers_.lcdc;
        case statAddress: return static_cast<uint8_t>(registers_.stat | 0x80);
        case scyAddress: return registers_.scy;
        case scxAddress: return registers_.scx;
        case lyAddress: return registers_.ly;
        case lycAddress: return registers_.lyc;
        case oamDmaAddress: return oamDmaRegister_;
        case bgpAddress: return registers_.bgp;
        case obp0Address: return registers_.obp0;
        case obp1Address: return registers_.obp1;
        case wyAddress: return registers_.wy;
        case wxAddress: return registers_.wx;
        case vramBankAddress:
            return cgb_ ? static_cast<uint8_t>(0xFE | memory_.vramBank()) : 0xFF;
        case hdma1Address:
            return cgb_ ? static_cast<uint8_t>(0xE0 | ((hdmaSource_ >> 8) & 0x1F)) : 0xFF;
        case hdma2Address:
            return cgb_ ? static_cast<uint8_t>(hdmaSource_ & 0xF0) : 0xFF;
        case hdma3Address:
            return cgb_ ? static_cast<uint8_t>(0xE0 | ((hdmaDestination_ >> 8) & 0x1F)) : 0xFF;
        case hdma4Address:
            return cgb_ ? static_cast<uint8_t>(hdmaDestination_ & 0xF0) : 0xFF;
        case hdma5Address:
            return cgb_ ? hdma5_ : 0xFF;
        case bgPaletteSpecAddress:
            return cgb_ ? memory_.readPaletteSpec(false) : 0xFF;
        case bgPaletteDataAddress:
            if (!cgb_ || (registers_.mode() == 3 && registers_.lcdEnabled())) {
                return 0xFF;
            }
            return memory_.readPaletteData(false);
        case objPaletteSpecAddress:
            return cgb_ ? memory_.readPaletteSpec(true) : 0xFF;
        case objPaletteDataAddress:
            if (!cgb_ || (registers_.mode() == 3 && registers_.lcdEnabled())) {
                return 0xFF;
            }
            return memory_.readPaletteData(true);
        case objectPriorityAddress:
            return cgb_ ? static_cast<uint8_t>(0xFE | (registers_.opri & 0x01)) : 0xFF;
        default:
            break;
    }

    assert(false && "Wrong PPU address");
    return 0xFF;
}

void GbPpu::write(uint16_t address, const uint8_t& value)
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        if (vramAccessBlocked()) {
            return;
        }
        memory_.writeVram(address, value);
        return;
    }

    if (address >= OamStart && address < 0xFF00) {
        if (oamAccessBlocked()) {
            if (oamBugEnabled_) {
                triggerOamBugWrite(address);
            }
            return;
        }
        if (address <= OamEnd) {
            memory_.writeOam(address, value);
        }
        return;
    }

    switch (address) {
        case lcdcAddress:
            writeLcdc(value);
            return;
        case statAddress:
            writeStat(value);
            return;
        case scyAddress:
            registers_.scy = value;
            return;
        case scxAddress:
            registers_.scx = value;
            return;
        case lyAddress:
            writeLy(value);
            return;
        case lycAddress:
            registers_.lyc = value;
            if (registers_.lcdEnabled()) {
                updateLyc();
            }
            return;
        case oamDmaAddress:
            startOamDma(value);
            return;
        case bgpAddress:
            registers_.bgp = value;
            return;
        case obp0Address:
            registers_.obp0 = value;
            return;
        case obp1Address:
            registers_.obp1 = value;
            return;
        case wyAddress:
            registers_.wy = value;
            return;
        case wxAddress:
            registers_.wx = value;
            return;
        case vramBankAddress:
            if (cgb_) {
                memory_.setVramBank(value);
            }
            return;
        case hdma1Address:
            if (cgb_) {
                hdmaSource_ = static_cast<uint16_t>((hdmaSource_ & 0x00F0)
                    | ((value & 0x1F) << 8));
            }
            return;
        case hdma2Address:
            if (cgb_) {
                hdmaSource_ = static_cast<uint16_t>((hdmaSource_ & 0xFF00)
                    | (value & 0xF0));
            }
            return;
        case hdma3Address:
            if (cgb_) {
                hdmaDestination_ = static_cast<uint16_t>(0x8000
                    | ((value & 0x1F) << 8) | (hdmaDestination_ & 0x00F0));
            }
            return;
        case hdma4Address:
            if (cgb_) {
                hdmaDestination_ = static_cast<uint16_t>((hdmaDestination_ & 0xFF00)
                    | (value & 0xF0));
            }
            return;
        case hdma5Address:
            if (cgb_) {
                writeHdma5(value);
            }
            return;
        case bgPaletteSpecAddress:
            if (cgb_) {
                memory_.writePaletteSpec(false, value);
            }
            return;
        case bgPaletteDataAddress:
            if (cgb_ && !(registers_.mode() == 3 && registers_.lcdEnabled())) {
                memory_.writePaletteData(false, value);
            }
            return;
        case objPaletteSpecAddress:
            if (cgb_) {
                memory_.writePaletteSpec(true, value);
            }
            return;
        case objPaletteDataAddress:
            if (cgb_ && !(registers_.mode() == 3 && registers_.lcdEnabled())) {
                memory_.writePaletteData(true, value);
            }
            return;
        case objectPriorityAddress:
            if (cgb_) {
                registers_.opri = static_cast<uint8_t>(value & 0x01);
            }
            return;
        default:
            break;
    }

    assert(false && "Wrong PPU address");
}

void GbPpu::step()
{
    stepOamDma();
    stepHdma();
    stepTiming();
}

void GbPpu::reset()
{
    registers_ = PpuRegisters{};
    memory_.reset();
    frame_.fill(rgb(255, 255, 255));
    dot_ = 0;
    lineLength_ = 456;
    mode3Dots_ = 0;
    windowLine_ = 0;
    frameCount_ = 0;
    accessedOamRow_ = -1;
    statIrqLine_ = false;
    lcdJustEnabled_ = false;
    registers_.ly = 0;
    registers_.setMode(2);
    updateLyc();

    oamDmaActive_ = false;
    oamDmaRegister_ = 0x00;
    oamDmaSource_ = 0x0000;
    oamDmaPendingSource_ = 0x0000;
    oamDmaIndex_ = 0;
    oamDmaTicks_ = 0;
    oamDmaStartupTicks_ = 0;

    hdmaActive_ = false;
    hdmaHblank_ = false;
    hdma5_ = 0xFF;
    hdmaRemaining_ = 0;
    hdmaTicks_ = 0;
    hdmaSource_ = 0x0000;
    hdmaDestination_ = 0x8000;
}

uint8_t GbPpu::currentMode() const
{
    return registers_.mode();
}

bool GbPpu::isLcdEnable() const
{
    return registers_.lcdEnabled();
}

void GbPpu::setExternalRead(std::function<uint8_t(uint16_t)> reader)
{
    externalRead_ = std::move(reader);
}

void GbPpu::setCgb(bool cgb)
{
    cgb_ = cgb;
    renderer_ = PpuRenderer(cgb);
    fifo_.setCgb(cgb);
}

void GbPpu::setDmgCompatibilityPalette(const DmgCompatibilityPalette& palette)
{
    memory_.setDmgCompatibilityPalette(palette);
}

void GbPpu::clearDmgCompatibilityPalette()
{
    memory_.clearDmgCompatibilityPalette();
}


void GbPpu::triggerOamBugWrite(uint16_t address, int rowOffset)
{
    if (!oamBugEnabled_ || address < 0xFE00 || address >= 0xFF00
        || accessedOamRow_ < 8) {
        return;
    }
    const int row = accessedOamRow_ + rowOffset;
    if (row < 8 || row + 8 > static_cast<int>(OamSize)) {
        return;
    }

    memory_.writeOamByte(static_cast<uint8_t>(row),
        bitwiseGlitch(memory_.readOamByte(static_cast<uint8_t>(row)),
            memory_.readOamByte(static_cast<uint8_t>(row - 8)),
            memory_.readOamByte(static_cast<uint8_t>(row - 4))));
    memory_.writeOamByte(static_cast<uint8_t>(row + 1),
        bitwiseGlitch(memory_.readOamByte(static_cast<uint8_t>(row + 1)),
            memory_.readOamByte(static_cast<uint8_t>(row - 7)),
            memory_.readOamByte(static_cast<uint8_t>(row - 3))));
    for (int i = 2; i < 8; ++i) {
        memory_.writeOamByte(static_cast<uint8_t>(row + i),
            memory_.readOamByte(static_cast<uint8_t>(row - 8 + i)));
    }
}

void GbPpu::triggerOamBugRead(uint16_t address)
{
    if (!oamBugEnabled_ || address < 0xFE00 || address >= 0xFF00
        || accessedOamRow_ < 8) {
        return;
    }
    const int row = accessedOamRow_;
    if (row < 8 || row + 8 > static_cast<int>(OamSize)) {
        return;
    }

    const auto readWord = [this](int offset) {
        return static_cast<uint16_t>(
            memory_.readOamByte(static_cast<uint8_t>(offset))
            | (static_cast<uint16_t>(memory_.readOamByte(static_cast<uint8_t>(offset + 1))) << 8));
    };
    const auto writeWord = [this](int offset, uint16_t value) {
        memory_.writeOamByte(static_cast<uint8_t>(offset), static_cast<uint8_t>(value & 0xFF));
        memory_.writeOamByte(static_cast<uint8_t>(offset + 1), static_cast<uint8_t>(value >> 8));
    };

    if ((row & 0x18) == 0x10) {
        if (row < 0x10) {
            return;
        }
        const uint16_t a = readWord(row - 0x10);
        const uint16_t b = readWord(row - 0x08);
        const uint16_t c = readWord(row);
        const uint16_t d = readWord(row - 0x04);
        const uint16_t glitch = static_cast<uint16_t>((b & (a | c | d)) | (a & c & d));
        memory_.writeOamByte(static_cast<uint8_t>(row - 0x08),
            static_cast<uint8_t>(glitch & 0xFF));
        memory_.writeOamByte(static_cast<uint8_t>(row - 0x07),
            static_cast<uint8_t>(glitch >> 8));
        for (int i = 0; i < 8; ++i) {
            const uint8_t value = memory_.readOamByte(static_cast<uint8_t>(row - 0x08 + i));
            memory_.writeOamByte(static_cast<uint8_t>(row - 0x10 + i), value);
            memory_.writeOamByte(static_cast<uint8_t>(row + i), value);
        }
        return;
    }

    if ((row & 0x18) == 0x00) {
        if (row < 0x20) {
            return;
        }
        const uint16_t a = readWord(row);
        const uint16_t b = readWord(row - 0x02);
        const uint16_t c = readWord(row - 0x04);
        const uint16_t d = readWord(row - 0x08);
        const uint16_t e = readWord(row - 0x10);
        uint16_t glitch;
        if (row == 0x40) {
            glitch = bitwiseGlitchQuaternaryDmg(readWord(0), a, b, readWord(row - 0x06),
                c, readWord(row - 0x07), d, e);
        } else if (row == 0x20) {
            glitch = bitwiseGlitchTertiary2(a, b, c, d, e);
        } else if (row == 0x60) {
            glitch = bitwiseGlitchTertiary3(a, b, c, d, e);
        } else {
            glitch = bitwiseGlitchTertiary1(a, b, c, d, e);
        }
        writeWord(row - 0x04, glitch);
        for (int i = 0; i < 8; ++i) {
            const uint8_t value = memory_.readOamByte(static_cast<uint8_t>(row - 0x08 + i));
            memory_.writeOamByte(static_cast<uint8_t>(row - 0x10 + i), value);
            memory_.writeOamByte(static_cast<uint8_t>(row - 0x20 + i), value);
            memory_.writeOamByte(static_cast<uint8_t>(row + i), value);
        }
        return;
    }

    const uint16_t glitch = bitwiseGlitchReadWord(
        readWord(row), readWord(row - 8), readWord(row - 4));
    writeWord(row - 8, glitch);
    writeWord(row, glitch);
    for (int i = 0; i < 8; ++i) {
        memory_.writeOamByte(static_cast<uint8_t>(row + i),
            memory_.readOamByte(static_cast<uint8_t>(row - 8 + i)));
    }
}

void GbPpu::triggerOamBugReadIncrease(uint16_t address)
{
    if (!oamBugEnabled_ || address < 0xFE00 || address >= 0xFF00
        || accessedOamRow_ < 0x20 || accessedOamRow_ >= 0x98) {
        return;
    }
    const int row = accessedOamRow_;
    if (row + 8 > static_cast<int>(OamSize)) {
        return;
    }

    memory_.writeOamByte(static_cast<uint8_t>(row - 8),
        bitwiseGlitchReadIncrease(
            memory_.readOamByte(static_cast<uint8_t>(row - 0x10)),
            memory_.readOamByte(static_cast<uint8_t>(row - 0x08)),
            memory_.readOamByte(static_cast<uint8_t>(row)),
            memory_.readOamByte(static_cast<uint8_t>(row - 0x04))));
    memory_.writeOamByte(static_cast<uint8_t>(row - 7),
        bitwiseGlitchReadIncrease(
            memory_.readOamByte(static_cast<uint8_t>(row - 0x0F)),
            memory_.readOamByte(static_cast<uint8_t>(row - 0x07)),
            memory_.readOamByte(static_cast<uint8_t>(row + 0x01)),
            memory_.readOamByte(static_cast<uint8_t>(row - 0x03))));
    for (int i = 0; i < 8; ++i) {
        const uint8_t value = memory_.readOamByte(static_cast<uint8_t>(row - 8 + i));
        memory_.writeOamByte(static_cast<uint8_t>(row + i), value);
        memory_.writeOamByte(static_cast<uint8_t>(row - 0x10 + i), value);
    }
}

void GbPpu::stepOamDma()
{
    if (oamDmaStartupTicks_ > 0) {
        --oamDmaStartupTicks_;
        if (oamDmaStartupTicks_ == 0) {
            oamDmaSource_ = oamDmaPendingSource_;
            oamDmaIndex_ = 0;
            oamDmaTicks_ = 0;
            oamDmaActive_ = true;
        }
    }

    if (!oamDmaActive_) {
        return;
    }

    ++oamDmaTicks_;
    if (oamDmaTicks_ < 4) {
        return;
    }
    oamDmaTicks_ = 0;

    const uint8_t value = readDmaSource(static_cast<uint16_t>(oamDmaSource_ + oamDmaIndex_));
    memory_.writeOamByte(static_cast<uint8_t>(oamDmaIndex_), value);
    ++oamDmaIndex_;
    if (oamDmaIndex_ >= OamSize) {
        oamDmaActive_ = false;
    }
}

void GbPpu::stepHdma()
{
    if (!hdmaActive_ || hdmaHblank_) {
        return;
    }

    ++hdmaTicks_;
    if (hdmaTicks_ >= 32) {
        hdmaTicks_ = 0;
        transferHdmaBlock();
    }
}

void GbPpu::stepTiming()
{
    if (!registers_.lcdEnabled()) {
        registers_.ly = 0;
        dot_ = 0;
        lineLength_ = 456;
        mode3Dots_ = 0;
        windowLine_ = 0;
        lcdJustEnabled_ = false;
        registers_.setMode(0);
        updateStatIrq();
        return;
    }

    if (registers_.ly < 144) {
        if (lcdJustEnabled_ && registers_.ly == 0) {
            // The first line after the LCD is enabled starts in mode 0 and
            // skips the OAM scan.  The fetcher starts slightly early so the
            // line ends four dots ahead of a normal line.
            if (dot_ == 79) {
                setMode(3);
                mode3Dots_ = 0;
                fifo_.startLine(registers_, memory_, registers_.ly, windowLine_);
            }
        } else if (dot_ == 0) {
            setMode(2);
            mode3Dots_ = 0;
            updateLyc();
        } else if (dot_ == 80) {
            setMode(3);
            mode3Dots_ = 0;
            fifo_.startLine(registers_, memory_, registers_.ly, windowLine_);
        }

        if (registers_.mode() == 3) {
            fifo_.step(registers_, memory_, frame_);
            if (mode3Dots_ < 255) {
                ++mode3Dots_;
            }
            if (fifo_.finished()) {
                finishMode3();
            }
        }
    } else if (dot_ == 0 && registers_.ly == 144) {
        setMode(1);
        if (interruptHandler_) {
            interruptHandler_->requestVBlankInterrupt();
        }
        ++frameCount_;
        if (display_) {
            display_->presentFrame(frame_);
        }
    }

    if (oamBugEnabled_ && registers_.lcdEnabled() && registers_.ly < 144
        && registers_.mode() == 2 && dot_ < 80) {
        const int index = dot_ / 2;
        accessedOamRow_ = ((index & ~1) * 4) + 8;
    } else {
        accessedOamRow_ = -1;
    }

    ++dot_;
    if (dot_ >= lineLength_) {
        dot_ = 0;
        lineLength_ = 456;
        ++registers_.ly;
        if (registers_.ly != 0) {
            lcdJustEnabled_ = false;
        }
        if (registers_.ly >= 154) {
            registers_.ly = 0;
            windowLine_ = 0;
        }
        if (registers_.ly >= 144 || registers_.ly == 0) {
            updateLyc();
        } else {
            // The LY=LYC comparator lags a little behind the LY increment;
            // the new result becomes visible at the start of the line.
            registers_.setLycCoincidence(false);
            updateStatIrq();
        }
    }
}

bool GbPpu::vramReadBlocked() const
{
    if (vramAccessBlocked()) {
        return true;
    }
    // As with OAM, reads lose access one dot before mode 3 becomes visible.
    return registers_.lcdEnabled() && registers_.ly < 144 && dot_ == 80;
}

void GbPpu::finishMode3()
{
    if (registers_.objEnabled()) {
        renderer_.renderSprites(registers_, memory_, registers_.ly,
            fifo_.backgroundColorIndex(), fifo_.backgroundPriority(), frame_);
    }
    if (fifo_.windowUsed()) {
        ++windowLine_;
    }
    setMode(0);
}

bool GbPpu::firstLineAccessBlocked() const
{
    if (!lcdJustEnabled_ || registers_.ly != 0) {
        return false;
    }
    if (registers_.mode() == 3) {
        return true;
    }
    // The PPU grabs the VRAM/OAM bus two dots before mode 3 starts on the
    // first line; once the first mode 3 has finished, access follows the
    // normal mode rules.
    return registers_.mode() == 0 && !fifo_.finished() && dot_ >= 78;
}

bool GbPpu::oamAccessBlocked() const
{
    if (!registers_.lcdEnabled()) {
        return oamDmaActive_;
    }
    if (firstLineAccessBlocked()) {
        return true;
    }
    if (registers_.mode() == 2) {
        // Hardware allows the final OAM-scan dot to slip through.
        return dot_ != 80;
    }
    if (registers_.mode() == 3) {
        return mode3Dots_ >= 2;
    }
    return oamDmaActive_;
}

bool GbPpu::oamReadBlocked() const
{
    if (!registers_.lcdEnabled()) {
        return oamDmaActive_;
    }
    if (firstLineAccessBlocked()) {
        return true;
    }
    // Read access is cut one dot before mode 2 becomes visible, while writes
    // are still accepted on that boundary dot.
    if (registers_.ly < 144 && dot_ == 0) {
        return true;
    }
    return registers_.mode() == 2 || registers_.mode() == 3 || oamDmaActive_;
}

bool GbPpu::vramAccessBlocked() const
{
    if (!registers_.lcdEnabled()) {
        return false;
    }
    if (firstLineAccessBlocked()) {
        return true;
    }
    return registers_.mode() == 3;
}

void GbPpu::setMode(uint8_t mode)
{
    registers_.setMode(mode);
    if (mode == 0 && hdmaActive_ && hdmaHblank_) {
        transferHdmaBlock();
    }
    updateStatIrq();
}

void GbPpu::updateLyc()
{
    const bool equal = registers_.ly == registers_.lyc;
    registers_.setLycCoincidence(equal);
    updateStatIrq();
}

void GbPpu::updateStatIrq()
{
    const uint8_t mode = registers_.mode();
    const bool signal = ((registers_.stat & StatBits::mode0Interrupt) && mode == 0)
        || ((registers_.stat & StatBits::mode1Interrupt) && mode == 1)
        || ((registers_.stat & StatBits::mode2Interrupt) && mode == 2)
        // DMG also raises the mode-2 STAT line at the start of VBlank.
        || ((registers_.stat & StatBits::mode2Interrupt)
            && mode == 1 && registers_.ly == 144)
        || ((registers_.stat & StatBits::lycInterrupt) && registers_.lycCoincidence());

    if (signal && !statIrqLine_) {
        requestLcdInterrupt();
    }
    statIrqLine_ = signal;
}

void GbPpu::requestLcdInterrupt()
{
    if (interruptHandler_) {
        interruptHandler_->requestLcdInterrupt();
    }
}

void GbPpu::startOamDma(uint8_t source)
{
    oamDmaRegister_ = source;
    oamDmaPendingSource_ = static_cast<uint16_t>(source) << 8;
    // The new transfer starts two M-cycles after the FF46 write.  If another
    // transfer is already running it keeps the OAM bus for those cycles.
    oamDmaStartupTicks_ = 8;
}

uint8_t GbPpu::readDmaSource(uint16_t address) const
{
    uint16_t source = address;
    if (source >= 0xE000 && source <= 0xFDFF) {
        source = static_cast<uint16_t>(source - 0x2000);
    }
    // On DMG hardware, OAM DMA sources in $FE00-$FFFF read the
    // corresponding WRAM echo at $DE00-$DFFF.  CGB hardware behaves
    // differently (Mooneye's sources-GS is a DMG-only pass).
    if (oamBugEnabled_ && source >= 0xFE00) {
        source = static_cast<uint16_t>(source - 0x2000);
    }
    if (source >= VideoRamStart && source <= VideoRamEnd) {
        return memory_.readVram(source);
    }
    if (source >= OamStart && source <= OamEnd) {
        return memory_.readOam(source);
    }
    if (externalRead_) {
        return externalRead_(source);
    }
    return 0xFF;
}

void GbPpu::transferHdmaBlock()
{
    if (!hdmaActive_ || hdmaRemaining_ == 0) {
        return;
    }

    for (uint8_t index = 0; index < 0x10; ++index) {
        const uint16_t source = static_cast<uint16_t>(hdmaSource_ + index);
        const uint16_t destination = static_cast<uint16_t>(
            0x8000 | ((hdmaDestination_ + index) & 0x1FFF));
        memory_.writeVram(destination, readDmaSource(source));
    }

    hdmaSource_ = static_cast<uint16_t>(hdmaSource_ + 0x10);
    hdmaDestination_ = static_cast<uint16_t>(0x8000 | ((hdmaDestination_ + 0x10) & 0x1FFF));
    --hdmaRemaining_;
    if (hdmaRemaining_ == 0) {
        hdmaActive_ = false;
        hdma5_ = static_cast<uint8_t>(hdma5_ | 0x80);
    } else {
        hdma5_ = static_cast<uint8_t>((hdma5_ & 0x80) | (hdmaRemaining_ - 1));
    }
}

void GbPpu::writeHdma5(uint8_t value)
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
}

void GbPpu::writeLcdc(uint8_t value)
{
    const bool wasEnabled = registers_.lcdEnabled();
    registers_.lcdc = value;
    const bool isEnabled = registers_.lcdEnabled();
    if (wasEnabled == isEnabled) {
        return;
    }

    registers_.ly = 0;
    dot_ = 0;
    lineLength_ = 456;
    mode3Dots_ = 0;
    windowLine_ = 0;
    if (isEnabled) {
        lcdJustEnabled_ = true;
        // The first line after enabling the LCD skips OAM scan and ends four
        // dots early.
        lineLength_ = 452;
        updateLyc();
        registers_.setMode(0);
        updateStatIrq();
    } else {
        lcdJustEnabled_ = false;
        registers_.setMode(0);
        // LY=LYC is retained while the comparison clock is stopped.
        updateStatIrq();
    }
}

void GbPpu::writeStat(uint8_t value)
{
    registers_.stat = static_cast<uint8_t>((value & 0x78) | (registers_.stat & 0x07));
    updateStatIrq();
}

void GbPpu::writeLy(uint8_t)
{
    registers_.ly = 0;
    dot_ = 0;
    if (registers_.lcdEnabled()) {
        updateLyc();
    }
}

}  // namespace qnemu

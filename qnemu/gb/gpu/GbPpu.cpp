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

GbPpu::GbPpu(bool cgb,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler) :
    cgb_(cgb),
    display_(std::move(display)),
    interruptHandler_(std::move(interruptHandler)),
    renderer_(cgb)
{
    GbPpu::reset();
}

uint8_t GbPpu::read(uint16_t address) const
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        if (registers_.mode() == 3 && registers_.lcdEnabled()) {
            return 0xFF;
        }
        return memory_.readVram(address);
    }

    if (address >= OamStart && address <= OamEnd) {
        if (((registers_.mode() == 2 || registers_.mode() == 3) && registers_.lcdEnabled())
            || oamDmaActive_) {
            return 0xFF;
        }
        return memory_.readOam(address);
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
        if (registers_.mode() == 3 && registers_.lcdEnabled()) {
            return;
        }
        memory_.writeVram(address, value);
        return;
    }

    if (address >= OamStart && address <= OamEnd) {
        if (((registers_.mode() == 2 || registers_.mode() == 3) && registers_.lcdEnabled())
            || oamDmaActive_) {
            return;
        }
        memory_.writeOam(address, value);
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
    mode0StartDot_ = 252;
    windowLine_ = 0;
    frameCount_ = 0;
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
        mode0StartDot_ = 252;
        windowLine_ = 0;
        lcdJustEnabled_ = false;
        registers_.setMode(0);
        updateStatIrq();
        return;
    }

    if (registers_.ly < 144) {
        if (lcdJustEnabled_ && registers_.ly == 0) {
            // The first line after LCD enable starts in mode 0 and skips the
            // normal OAM-scan phase.
            if (dot_ == 0) {
                setMode(0);
            } else if (dot_ == 20) {
                setMode(3);
                renderScanline();
                mode0StartDot_ = static_cast<uint16_t>(20 + mode3Length());
            } else if (dot_ == mode0StartDot_) {
                setMode(0);
            }
        } else if (dot_ == 0) {
            setMode(2);
        } else if (dot_ == 80) {
            setMode(3);
            renderScanline();
            mode0StartDot_ = static_cast<uint16_t>(80 + mode3Length());
        } else if (dot_ == mode0StartDot_) {
            setMode(0);
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

    ++dot_;
    if (dot_ >= 456) {
        dot_ = 0;
        ++registers_.ly;
        if (registers_.ly != 0) {
            lcdJustEnabled_ = false;
        }
        if (registers_.ly >= 154) {
            registers_.ly = 0;
            windowLine_ = 0;
            mode0StartDot_ = 252;
        }
        updateLyc();
    }
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

uint16_t GbPpu::mode3Length() const
{
    uint16_t length = 172;
    const uint8_t fineScroll = static_cast<uint8_t>(registers_.scx & 0x07);
    if (fineScroll != 0) {
        // The fetcher/STAT alignment used by the hardware timing tests makes
        // non-zero fine scroll consume an extra 4 dots, and 8 for the last
        // three fine-scroll positions.
        length = static_cast<uint16_t>(length + (fineScroll <= 4 ? 4 : 8));
    }

    const bool windowVisible = registers_.windowEnabled()
        && registers_.wx >= 7 && registers_.wx <= 166
        && registers_.ly >= registers_.wy;
    if (windowVisible) {
        length = static_cast<uint16_t>(length + 6);
    }

    const int spriteHeight = registers_.objSize16() ? 16 : 8;
    std::array<bool, 21> tilesSeen{};
    bool x0ExtraApplied = false;
    int spriteCount = 0;
    for (uint8_t index = 0; index < OamSize && spriteCount < 10; index += 4) {
        const int spriteY = static_cast<int>(memory_.readOamByte(index)) - 16;
        if (static_cast<int>(registers_.ly) < spriteY
            || static_cast<int>(registers_.ly) >= spriteY + spriteHeight) {
            continue;
        }

        const uint8_t oamX = memory_.readOamByte(static_cast<uint8_t>(index + 1));
        const int screenX = static_cast<int>(oamX) - 8;
        if (screenX >= 160) {
            continue;
        }

        ++spriteCount;
        int penalty = 6;
        if (oamX == 0) {
            // The left-edge object has one extra alignment penalty, but only
            // once even when several objects share X=0.
            if (!x0ExtraApplied) {
                penalty += 5;
                x0ExtraApplied = true;
            }
        } else if (screenX >= 0) {
            const int scrolledX = screenX + registers_.scx;
            const int tile = std::min(20, scrolledX >> 3);
            if (!tilesSeen.at(static_cast<std::size_t>(tile))) {
                const int pixel = scrolledX & 0x07;
                penalty += std::max(0, (7 - pixel) - 2);
                tilesSeen.at(static_cast<std::size_t>(tile)) = true;
            }
        }
        length = static_cast<uint16_t>(length + penalty);
    }

    return std::min<uint16_t>(length, 289);
}

void GbPpu::requestLcdInterrupt()
{
    if (interruptHandler_) {
        interruptHandler_->requestLcdInterrupt();
    }
}

void GbPpu::renderScanline()
{
    if (renderer_.renderLine(registers_, memory_, registers_.ly, windowLine_, frame_)) {
        ++windowLine_;
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
    mode0StartDot_ = 252;
    windowLine_ = 0;
    if (isEnabled) {
        lcdJustEnabled_ = true;
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

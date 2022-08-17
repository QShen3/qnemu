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

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/gpu/Mode.h"
#include "qnemu/gb/gpu/SpriteAttributeTable.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

class GbGpu : public GbGpuInterface
{
public:
    GbGpu() = delete;
    GbGpu(const GbCartridgeInterface& cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::unique_ptr<GbcPalette> gbcPalette,
        std::unique_ptr<SpriteAttributeTable> spriteAttributeTable);
    ~GbGpu();

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    union GbcTileAttribute {
        struct {
            uint backgroundPaletteNumber : 3;
            uint tileVideoRamBank : 1;
            uint : 1;
            uint horizontalFlip : 1;
            uint verticalFlip : 1;
            uint backgroundToOAMPriority : 1;
        };
        uint8_t attribute;
    };
    union GbcSpriteAttribute {
        struct {
            uint gbcPaletteNumber : 3;
            uint tileVideoRamBank : 1;
            uint paletteNumber : 1;
            uint horizontalFlip : 1;
            uint verticalFlip : 1;
            uint backgroundToOAMPriority : 1;
        };
        uint8_t attribute;
    };

    void checklcdYCoordinate();
    std::tuple<uint16_t, bool> getColorIndexAndPriorityOfBackgroundOrWindow(uint8_t x, uint8_t y, size_t tileMapOffset) const;
    QRgb getGbColor(uint16_t colorIndex, uint8_t paletteData) const;
    void renderLine();
    void scanSprites();

    struct {
        union {
            struct {
                uint8_t backgroundAndWindowPriority : 1;
                uint8_t spriteEnable : 1;
                uint8_t spriteSize : 1;
                uint8_t backgroundTileMapArea : 1;
                uint8_t backgroundAndWindowTileDataArea : 1;
                uint8_t windowEnable : 1;
                uint8_t windowTileMapArea : 1;
                uint8_t lcdEnable : 1;
            };
            uint8_t lcdControl;
        };  // FF40
        union {
            struct {
                uint8_t modeFlag : 2;
                uint8_t coincidenceFlag : 1;
                uint8_t mode0HBlankInterrupt : 1;
                uint8_t mode1VBlankInterrupt : 1;
                uint8_t mode2OAMInterrupt : 1;
                uint8_t lyCoincidenceInterrupt : 1;
            };
            uint8_t lcdStatus;
        };  // FF41
        uint8_t scrollY;  // FF42
        uint8_t scrollX;  // FF43
        uint8_t lcdYCoordinate;  // FF44
        uint8_t lcdYCoordinateCompare;  // FF45
        union {
            struct {
                uint8_t shadeForBackgroundColorIndex0 : 2;
                uint8_t shadeForBackgroundColorIndex1 : 2;
                uint8_t shadeForBackgroundColorIndex2 : 2;
                uint8_t shadeForBackgroundColorIndex3 : 2;
            };
            uint8_t backgroundPaletteData;
        };  // FF47
        union {
            struct {
                uint8_t shadeForSprite0ColorIndex0 : 2;
                uint8_t shadeForSprite0ColorIndex1 : 2;
                uint8_t shadeForSprite0ColorIndex2 : 2;
                uint8_t shadeForSprite0ColorIndex3 : 2;
            };
            uint8_t spritePalette0Data;
        };  // FF48
        union {
            struct {
                uint8_t shadeForSprite1ColorIndex0 : 2;
                uint8_t shadeForSprite1ColorIndex1 : 2;
                uint8_t shadeForSprite1ColorIndex2 : 2;
                uint8_t shadeForSprite1ColorIndex3 : 2;
            };
            uint8_t spritePalette1Data;
        };  // FF49
        uint8_t windowYPosition;  // FF4A
        uint8_t windowXPosition;  // FF4B
        uint8_t videoRamBank;  // FF4F
        uint8_t newDMASourceHigh;  // FF51
        uint8_t newDMASourceLow;  // FF52
        uint8_t newDMADestinationHigh;  // FF53
        uint8_t newDMADestinationLow;  // FF54
        uint8_t newDMALength;  // FF55
    } registers;
    std::array<std::array<bool, 144>, 160> backgroundToOAMPriorityMap;
    const GbCartridgeInterface& cartridge;
    std::array<std::array<uint8_t, 144>, 160> colorIndexMap;
    std::shared_ptr<DisplayInterface> display;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    std::unique_ptr<GbcPalette> gbcPalette;
    std::unique_ptr<SpriteAttributeTable> spriteAttributeTable;
    std::stack<uint8_t> spriteStack;
    std::vector<std::reference_wrapper<GbDeviceInterface>> subDevices;
    uint16_t ticks;
    std::array<std::array<uint8_t, VideoRamBankSize>, 2> videoRamBanks;
    uint16_t windowLineCounter;
    const std::array<Mode, 4> modes;

    // Modes
    void mode0();
    void mode1();
    void mode2();
    void mode3();
};

}  // namespace qnemu
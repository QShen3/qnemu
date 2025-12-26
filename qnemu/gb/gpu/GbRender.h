/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stack>
#include <tuple>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/gpu/GbPalette.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/GbOam.h"
#include "qnemu/gb/gpu/GbVideoRam.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

class GbRender : public GbDeviceInterface
{
public:
    GbRender() = delete;
    GbRender(const GbCartridgeInterface& cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        const GbPalette& gbPalette,
        const GbcPalette& gbcPalette,
        const GbOam& oam,
        const GbVideoRam& videoRam);
    ~GbRender() override= default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t currentMode() const;
    bool isLcdEnable() const;

    void horizontalBlank();
    void verticalBlank();
    void scanSprites();
    void renderLine();

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
        uint8_t windowYPosition;  // FF4A
        uint8_t windowXPosition;  // FF4B
    } registers;

    const GbCartridgeInterface& cartridge;
    std::shared_ptr<DisplayInterface> display;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    const GbPalette& gbPalette;
    const GbcPalette& gbcPalette;
    const GbOam& oam;
    const GbVideoRam& videoRam;

    std::stack<uint8_t> spriteStack;
    std::array<std::array<bool, 144>, 160> backgroundToOAMPriorityMap;
    std::array<std::array<uint8_t, 144>, 160> colorIndexMap;
    uint16_t windowLineCounter;

    void checklcdYCoordinate();
    std::tuple<uint16_t, bool> getColorIndexAndPriorityOfBackgroundOrWindow(uint8_t x, uint8_t y, size_t tileMapOffset) const;
};

}  // namespace qnemu
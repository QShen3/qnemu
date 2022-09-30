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
#include "qnemu/gb/gpu/GbPalette.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/gpu/GbVideoRam.h"
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
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler);
    ~GbGpu();

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t currentMode() const override;
    bool isLcdEnable() const override;

    void addGbPalette(std::unique_ptr<GbPalette> gbPalette);
    void addGbcPalette(std::unique_ptr<GbcPalette> gbcPalette);
    void addSpriteAttributeTable(std::unique_ptr<SpriteAttributeTable> spriteAttributeTable);
    void addGbVideoRam(std::unique_ptr<GbVideoRam> gbVideoRam);

private:
    friend GbPalette;
    friend GbcPalette;
    friend SpriteAttributeTable;
    friend GbVideoRam;
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
        uint8_t windowYPosition;  // FF4A
        uint8_t windowXPosition;  // FF4B
    } registers;
    std::array<std::array<bool, 144>, 160> backgroundToOAMPriorityMap;
    const GbCartridgeInterface& cartridge;
    std::array<std::array<uint8_t, 144>, 160> colorIndexMap;
    std::shared_ptr<DisplayInterface> display;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    std::unique_ptr<GbPalette> gbPalette;
    std::unique_ptr<GbcPalette> gbcPalette;
    std::unique_ptr<SpriteAttributeTable> spriteAttributeTable;
    std::unique_ptr<GbVideoRam> gbVideoRam;
    std::stack<uint8_t> spriteStack;
    std::vector<std::reference_wrapper<GbDeviceInterface>> subDevices;
    uint16_t ticks;
    uint16_t windowLineCounter;
    const std::array<Mode, 4> modes;

    // Modes
    void mode0();
    void mode1();
    void mode2();
    void mode3();
};

}  // namespace qnemu
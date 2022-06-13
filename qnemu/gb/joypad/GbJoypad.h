/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemu
{

class GbJoypad : public GbDeviceInterface
{
public:
    GbJoypad() = delete;
    GbJoypad(std::shared_ptr<DisplayInterface> display, std::shared_ptr<GbInterruptHandler> interruptHander);
    ~GbJoypad() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;
private:
    void processKeyPressEvent(int key);
    void processKeyReleaseEvent(int key);
    mutable struct {
        union {
            struct {
                uint8_t rightOrA : 1;
                uint8_t leftOrB : 1;
                uint8_t upOrSelect : 1;
                uint8_t downOrStart : 1;
                uint8_t selectDirectionButtons : 1;
                uint8_t selectActionButtons : 1;
                uint8_t : 2;
            };
            uint8_t joypadState;
        };
    } registers;
    mutable std::mutex mutex;
    union {
        struct {
            uint8_t a : 1;
            uint8_t b : 1;
            uint8_t select : 1;
            uint8_t start : 1;
            uint8_t : 4;
        };
        uint8_t data;
    } actionButtonsState;
    union {
        struct {
            uint8_t right : 1;
            uint8_t left : 1;
            uint8_t up : 1;
            uint8_t down : 1;
            uint8_t : 4;
        };
        uint8_t data;
    } directionButtionState;
    std::shared_ptr<DisplayInterface> display;
    std::shared_ptr<GbInterruptHandler> interruptHandler;
};

}  // namespace qnemu

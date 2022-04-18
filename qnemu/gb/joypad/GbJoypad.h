/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbJoypad : public GbDeviceInterface
{
public:
    GbJoypad();
    ~GbJoypad() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;
private:
    struct {
        union {
            struct {
                uint8_t rightOrA : 1;
                uint8_t leftOrB : 1;
                uint8_t upOrSelect : 1;
                uint8_t downOrStart : 1;
                uint8_t selectDirectionButtions : 1;
                uint8_t selectActionButtons : 1;
                uint8_t : 2;
            };
            uint8_t joypadState;
        };
    } registers;
};

}  // namespace qnemu
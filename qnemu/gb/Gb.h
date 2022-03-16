/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"

namespace qnemu
{

class Gb
{
public:
    Gb();
    ~Gb() = default;

    void loadCartridge(const char* filePath);

private:
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::shared_ptr<GbCpuInterface> cpu;
};

}  // namespace qnemu
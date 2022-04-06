/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"

namespace qnemu
{

class Gb
{
public:
    Gb();
    ~Gb() = default;

    void loadCartridge(const char* filePath);
    void setDisplay(std::shared_ptr<DisplayInterface> display);

private:
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::shared_ptr<GbCpuInterface> cpu;
    const GbMbcFactory mbcFactory;
    std::shared_ptr<GbGpuInterface> gpu;
};

}  // namespace qnemu
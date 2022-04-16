/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include <QtGui/QWindow>

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
    std::shared_ptr<QWindow> getDisplay();
    void stop();

private:
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::shared_ptr<GbCpuInterface> cpu;
    const GbMbcFactory mbcFactory;
    std::shared_ptr<QWindow> display;
};

}  // namespace qnemu
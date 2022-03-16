/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbMbcInterface : public GbDeviceInterface
{
public:
    static constexpr size_t romBankSize = 0x4000;
    static constexpr size_t ramBankSize = 0x2000;
};

}  // namespace qnemu
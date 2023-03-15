/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/MmuInterface.h"

namespace qnemu
{

class GbMmuInterface : public MmuInterface<uint8_t, uint16_t>
{

};

}  // namespace qnemu
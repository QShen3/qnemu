/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace qnemu
{

struct Instruction
{
    const char* disassembly;
    uint8_t length;
    uint8_t ticks;
    std::function<void()> execute;
};

}  // namespace qnemu
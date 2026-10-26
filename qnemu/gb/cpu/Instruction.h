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
    const char* disassembly = nullptr;
    uint8_t length = 0;
    uint8_t ticks = 0;
    std::function<void()> execute = {};
};

}  // namespace qnemu
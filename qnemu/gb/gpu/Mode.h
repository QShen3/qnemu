/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace qnemu
{

struct Mode
{
    std::string name;
    uint16_t ticks;
    std::function<void()> execute;
};

}  // namespace qnemu
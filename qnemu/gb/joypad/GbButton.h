/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

/// Game Boy's eight joypad buttons, independent of any host key mapping.
enum class GbButton : uint8_t
{
    right,
    left,
    up,
    down,
    a,
    b,
    select,
    start,
};

}  // namespace qnemu

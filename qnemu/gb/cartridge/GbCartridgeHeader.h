/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace qnemu
{

struct GbCartridgeHeader
{
    std::string title;
    std::string publisher;
    std::string region;
    std::array<uint8_t, 16> titleBytes {};
    uint8_t oldLicensee = 0;
    std::array<uint8_t, 2> newLicensee {};
    bool cgb = false;
    uint8_t type = 0;
};

}  // namespace qnemu

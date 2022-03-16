/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <memory>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"

namespace qnemu
{

class GbMbcFactoryInterface
{
    public:
        virtual ~GbMbcFactoryInterface() = default;

        virtual std::unique_ptr<GbMbcInterface> create(
            std::vector<std::array<uint8_t, GbMbcInterface::romBankSize>>&& romBanks,
            std::vector<std::array<uint8_t, GbMbcInterface::ramBankSize>>&& ramBanks,
            uint8_t type) const = 0;
};

}  // namespace qnemu

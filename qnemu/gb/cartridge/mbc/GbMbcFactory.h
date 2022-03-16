/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"

namespace qnemu
{

class GbMbcFactory : public GbMbcFactoryInterface
{
    public:
        GbMbcFactory() = default;
        ~GbMbcFactory() = default;

        std::unique_ptr<GbMbcInterface> create(std::vector<std::array<uint8_t, GbMbcInterface::romBankSize>>&& romBanks,
            std::vector<std::array<uint8_t, GbMbcInterface::ramBankSize>>&& ramBanks, uint8_t type) const override;
};

}  // namespace qnemu


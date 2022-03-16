/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbCircuitMbc.h"
#include "qnemu/gb/cartridge/mbc/GbMbc1.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"

namespace qnemu
{

std::unique_ptr<GbMbcInterface> GbMbcFactory::create(std::vector<std::array<uint8_t, GbMbcInterface::romBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, GbMbcInterface::ramBankSize>>&& ramBanks, uint8_t type) const
{
    std::unique_ptr<GbMbcInterface> mbc;

    if (type == 0x0 || type == 0x8 || type == 0x9) {
        mbc = std::make_unique<GbCircuitMbc>(std::move(romBanks), std::move(ramBanks), type);
    } else if (type == 0x1 || type == 0x2 || type == 0x3) {
        mbc = std::make_unique<GbMbc1>(std::move(romBanks), std::move(ramBanks), type);
    }
    // else if (type == 0x5 || type == 0x6) {
    //     mbc = std::make_unique<GbMbc2>(std::move(romBanks), std::move(ramBanks), type);
    // }
    // else if (type == 0xF || type == 0x10 || type == 0x11 || type == 0x12 || type == 0x13) {
    //     mbc = std::make_unique<GbMbc3>(std::move(romBanks), std::move(ramBanks), type);
    // }
    // else if (type == 0x19 || type == 0x1A || type == 0x1B || type == 0x1C || type == 0x1D || type == 0x1E) {
    //     mbc = std::make_unique<GbMbc5>(std::move(romBanks), std::move(ramBanks), type);
    // }
    // else if (type == 0x20) {
    //     mbc = std::make_unique<GbMbc6>(std::move(romBanks), std::move(ramBanks), type);
    //     // MBC6 is not complete
    //     throw std::runtime_error("This game is not supported!");
    // }
    else {
        throw std::runtime_error("This game is not supported!");
    }

    return mbc;
}

}  // namespace qnemu

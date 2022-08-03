/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>

#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/gpu/SpriteAttributeTable.h"

namespace qnemu
{

SpriteAttributeTable::SpriteAttributeTable(std::shared_ptr<GbCpuInterface> cpu) : cpu(cpu)
{
    SpriteAttributeTable::reset();
}

bool SpriteAttributeTable::accepts(uint16_t address) const
{
    if (address >= 0xFE00 && address < 0xFEA0) {
        return true;
    }
    if (address == 0xFF46) {
        return true;
    }
    return false;
}

uint8_t SpriteAttributeTable::read(uint16_t address) const
{
    if (address >= 0xFE00 && address < 0xFEA0) {
        return data.at(address - 0xFE00);
    }
    if (address == 0xFF46) {
        return registers.dmaTransferAndStartAddress;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void SpriteAttributeTable::write(uint16_t address, const uint8_t& value)
{
    if (address >= 0xFE00 && address < 0xFEA0) {
        data.at(address - 0xFE00) = value;
    }
    if (address == 0xFF46) {
        registers.dmaTransferAndStartAddress = value;
        if (!isDmaInProgress) {
            isDmaInProgress = true;
            dmaTicks = 640;
        }
    }
}

void SpriteAttributeTable::step()
{
    if (isDmaInProgress) {
        dmaTicks--;
        if (dmaTicks == 0) {
            isDmaInProgress = false;
            for (uint8_t i = 0; i < 0xA0; i++) {
                data.at(i) = cpu.lock()->readByte(registers.dmaTransferAndStartAddress * 0x100 + i);
            }
        }
    }
}

void SpriteAttributeTable::reset()
{
    std::memset(&registers, 0, sizeof(registers));

    isDmaInProgress = false;
    dmaTicks = 0;
}

uint8_t SpriteAttributeTable::at(uint16_t address) const
{
    return data.at(address);
}

}  // namespace qnemu
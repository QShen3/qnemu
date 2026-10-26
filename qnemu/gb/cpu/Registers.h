/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

namespace CpuFlags
{
inline constexpr uint8_t zero = 0x80;
inline constexpr uint8_t subtract = 0x40;
inline constexpr uint8_t halfCarry = 0x20;
inline constexpr uint8_t carry = 0x10;
}  // namespace CpuFlags

/**
 * SM83 register file.
 *
 * Deliberately a plain struct instead of anonymous unions/bit-fields: the
 * byte order of the register pairs is then identical on every compiler, the
 * low nibble of F is easy to mask, and a test can snapshot the whole state
 * without depending on implementation-defined layout.
 */
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Registers
{
    uint8_t a = 0x11;
    uint8_t f = CpuFlags::zero;
    uint8_t b = 0x00;
    uint8_t c = 0x00;
    uint8_t d = 0xFF;
    uint8_t e = 0x56;
    uint8_t h = 0x00;
    uint8_t l = 0x0D;
    uint16_t sp = 0xFFFE;
    uint16_t pc = 0x0100;

    uint16_t af() const { return (static_cast<uint16_t>(a) << 8) | f; }
    uint16_t bc() const { return (static_cast<uint16_t>(b) << 8) | c; }
    uint16_t de() const { return (static_cast<uint16_t>(d) << 8) | e; }
    uint16_t hl() const { return (static_cast<uint16_t>(h) << 8) | l; }

    void setAf(uint16_t value)
    {
        a = static_cast<uint8_t>(value >> 8);
        f = static_cast<uint8_t>(value & 0xF0);
    }
    void setBc(uint16_t value)
    {
        b = static_cast<uint8_t>(value >> 8);
        c = static_cast<uint8_t>(value & 0xFF);
    }
    void setDe(uint16_t value)
    {
        d = static_cast<uint8_t>(value >> 8);
        e = static_cast<uint8_t>(value & 0xFF);
    }
    void setHl(uint16_t value)
    {
        h = static_cast<uint8_t>(value >> 8);
        l = static_cast<uint8_t>(value & 0xFF);
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace qnemu

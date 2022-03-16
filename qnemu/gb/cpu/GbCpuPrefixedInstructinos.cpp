/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/cpu/GbCpu.h"

namespace qnemu
{

void GbCpu::rlc_b()
{
    rlc(registers.b);
}

void GbCpu::rlc_c()
{
    rlc(registers.c);
}

void GbCpu::rlc_d()
{
    rlc(registers.d);
}

void GbCpu::rlc_e()
{
    rlc(registers.e);
}

void GbCpu::rlc_h()
{
    rlc(registers.h);
}

void GbCpu::rlc_l()
{
    rlc(registers.l);
}

void GbCpu::rlc_hlp()
{
    uint8_t value = readByte(registers.hl);
    rlc(value);
    writeByte(registers.hl, value);
}

void GbCpu::rlc_a()
{
    rlc(registers.a);
}

void GbCpu::rrc_b()
{
    rrc(registers.b);
}

void GbCpu::rrc_c()
{
    rrc(registers.c);
}

void GbCpu::rrc_d()
{
    rrc(registers.d);
}

void GbCpu::rrc_e()
{
    rrc(registers.e);
}

void GbCpu::rrc_h()
{
    rrc(registers.h);
}

void GbCpu::rrc_l()
{
    rrc(registers.l);
}

void GbCpu::rrc_hlp()
{
    uint8_t value = readByte(registers.hl);
    rrc(value);
    writeByte(registers.hl, value);
}

void GbCpu::rrc_a()
{
    rrc(registers.a);
}

void GbCpu::rl_b()
{
    rl(registers.b);
}

void GbCpu::rl_c()
{
    rl(registers.c);
}

void GbCpu::rl_d()
{
    rl(registers.d);
}

void GbCpu::rl_e()
{
    rl(registers.e);
}

void GbCpu::rl_h()
{
    rl(registers.h);
}

void GbCpu::rl_l()
{
    rl(registers.l);
}

void GbCpu::rl_hlp()
{
    uint8_t value = readByte(registers.hl);
    rl(value);
    writeByte(registers.hl, value);
}

void GbCpu::rl_a()
{
    rl(registers.a);
}

void GbCpu::rr_b()
{
    rr(registers.b);
}

void GbCpu::rr_c()
{
    rr(registers.c);
}

void GbCpu::rr_d()
{
    rr(registers.d);
}

void GbCpu::rr_e()
{
    rr(registers.e);
}

void GbCpu::rr_h()
{
    rr(registers.h);
}

void GbCpu::rr_l()
{
    rr(registers.l);
}

void GbCpu::rr_hlp()
{
    uint8_t value = readByte(registers.hl);
    rr(value);
    writeByte(registers.hl, value);
}

void GbCpu::rr_a()
{
    rr(registers.a);
}

void GbCpu::sla_b()
{
    sla(registers.b);
}

void GbCpu::sla_c()
{
    sla(registers.c);
}

void GbCpu::sla_d()
{
    sla(registers.d);
}

void GbCpu::sla_e()
{
    sla(registers.e);
}

void GbCpu::sla_h()
{
    sla(registers.h);
}

void GbCpu::sla_l()
{
    sla(registers.l);
}

void GbCpu::sla_hlp()
{
    uint8_t value = readByte(registers.hl);
    sla(value);
    writeByte(registers.hl, value);
}

void GbCpu::sla_a()
{
    sla(registers.a);
}

void GbCpu::sra_b()
{
    sra(registers.b);
}

void GbCpu::sra_c()
{
    sra(registers.c);
}

void GbCpu::sra_d()
{
    sra(registers.d);
}

void GbCpu::sra_e()
{
    sra(registers.e);
}

void GbCpu::sra_h()
{
    sra(registers.h);
}

void GbCpu::sra_l()
{
    sra(registers.l);
}

void GbCpu::sra_hlp()
{
    uint8_t value = readByte(registers.hl);
    sra(value);
    writeByte(registers.hl, value);
}

void GbCpu::sra_a()
{
    sra(registers.a);
}

void GbCpu::swap_b()
{
    swap(registers.b);
}

void GbCpu::swap_c()
{
    swap(registers.c);
}

void GbCpu::swap_d()
{
    swap(registers.d);
}

void GbCpu::swap_e()
{
    swap(registers.e);
}

void GbCpu::swap_h()
{
    swap(registers.h);
}

void GbCpu::swap_l()
{
    swap(registers.l);
}

void GbCpu::swap_hlp()
{
    uint8_t value = readByte(registers.hl);
    swap(value);
    writeByte(registers.hl, value);
}

void GbCpu::swap_a()
{
    swap(registers.a);
}

void GbCpu::srl_b()
{
    srl(registers.b);
}

void GbCpu::srl_c()
{
    srl(registers.c);
}

void GbCpu::srl_d()
{
    srl(registers.d);
}

void GbCpu::srl_e()
{
    srl(registers.e);
}

void GbCpu::srl_h()
{
    srl(registers.h);
}

void GbCpu::srl_l()
{
    srl(registers.l);
}

void GbCpu::srl_hlp()
{
    uint8_t value = readByte(registers.hl);
    srl(value);
    writeByte(registers.hl, value);
}

void GbCpu::srl_a()
{
    srl(registers.a);
}

void GbCpu::bit_0_b()
{
    bit(0x1, registers.b);
}

void GbCpu::bit_0_c()
{
    bit(0x1, registers.c);
}

void GbCpu::bit_0_d()
{
    bit(0x1, registers.d);
}

void GbCpu::bit_0_e()
{
    bit(0x1, registers.e);
}

void GbCpu::bit_0_h()
{
    bit(0x1, registers.h);
}

void GbCpu::bit_0_l()
{
    bit(0x1, registers.l);
}

void GbCpu::bit_0_hlp()
{
    bit(0x1, readByte(registers.hl));
}

void GbCpu::bit_0_a()
{
    bit(0x1, registers.a);
}

void GbCpu::bit_1_b()
{
    bit(0x2, registers.b);
}

void GbCpu::bit_1_c()
{
    bit(0x2, registers.c);
}

void GbCpu::bit_1_d()
{
    bit(0x2, registers.d);
}

void GbCpu::bit_1_e()
{
    bit(0x2, registers.e);
}

void GbCpu::bit_1_h()
{
    bit(0x2, registers.h);
}

void GbCpu::bit_1_l()
{
    bit(0x2, registers.l);
}

void GbCpu::bit_1_hlp()
{
    bit(0x2, readByte(registers.hl));
}

void GbCpu::bit_1_a()
{
    bit(0x2, registers.a);
}

void GbCpu::bit_2_b()
{
    bit(0x4, registers.b);
}

void GbCpu::bit_2_c()
{
    bit(0x4, registers.c);
}

void GbCpu::bit_2_d()
{
    bit(0x4, registers.d);
}

void GbCpu::bit_2_e()
{
    bit(0x4, registers.e);
}

void GbCpu::bit_2_h()
{
    bit(0x4, registers.h);
}

void GbCpu::bit_2_l()
{
    bit(0x4, registers.l);
}

void GbCpu::bit_2_hlp()
{
    bit(0x4, readByte(registers.hl));
}

void GbCpu::bit_2_a()
{
    bit(0x4, registers.a);
}

void GbCpu::bit_3_b()
{
    bit(0x8, registers.b);
}

void GbCpu::bit_3_c()
{
    bit(0x8, registers.c);
}

void GbCpu::bit_3_d()
{
    bit(0x8, registers.d);
}

void GbCpu::bit_3_e()
{
    bit(0x8, registers.e);
}

void GbCpu::bit_3_h()
{
    bit(0x8, registers.h);
}

void GbCpu::bit_3_l()
{
    bit(0x8, registers.l);
}

void GbCpu::bit_3_hlp()
{
    bit(0x8, readByte(registers.hl));
}

void GbCpu::bit_3_a()
{
    bit(0x8, registers.a);
}

void GbCpu::bit_4_b()
{
    bit(0x10, registers.b);
}

void GbCpu::bit_4_c()
{
    bit(0x10, registers.c);
}

void GbCpu::bit_4_d()
{
    bit(0x10, registers.d);
}

void GbCpu::bit_4_e()
{
    bit(0x10, registers.e);
}

void GbCpu::bit_4_h()
{
    bit(0x10, registers.h);
}

void GbCpu::bit_4_l()
{
    bit(0x10, registers.l);
}

void GbCpu::bit_4_hlp()
{
    bit(0x10, readByte(registers.hl));
}

void GbCpu::bit_4_a()
{
    bit(0x10, registers.a);
}

void GbCpu::bit_5_b()
{
    bit(0x20, registers.b);
}

void GbCpu::bit_5_c()
{
    bit(0x20, registers.c);
}

void GbCpu::bit_5_d()
{
    bit(0x20, registers.d);
}

void GbCpu::bit_5_e()
{
    bit(0x20, registers.e);
}

void GbCpu::bit_5_h()
{
    bit(0x20, registers.h);
}

void GbCpu::bit_5_l()
{
    bit(0x20, registers.l);
}

void GbCpu::bit_5_hlp()
{
    bit(0x20, readByte(registers.hl));
}

void GbCpu::bit_5_a()
{
    bit(0x20, registers.a);
}

void GbCpu::bit_6_b()
{
    bit(0x40, registers.b);
}

void GbCpu::bit_6_c()
{
    bit(0x40, registers.c);
}

void GbCpu::bit_6_d()
{
    bit(0x40, registers.d);
}

void GbCpu::bit_6_e()
{
    bit(0x40, registers.e);
}

void GbCpu::bit_6_h()
{
    bit(0x40, registers.h);
}

void GbCpu::bit_6_l()
{
    bit(0x40, registers.l);
}

void GbCpu::bit_6_hlp()
{
    bit(0x40, readByte(registers.hl));
}

void GbCpu::bit_6_a()
{
    bit(0x40, registers.a);
}

void GbCpu::bit_7_b()
{
    bit(0x80, registers.b);
}

void GbCpu::bit_7_c()
{
    bit(0x80, registers.c);
}

void GbCpu::bit_7_d()
{
    bit(0x80, registers.d);
}

void GbCpu::bit_7_e()
{
    bit(0x80, registers.e);
}

void GbCpu::bit_7_h()
{
    bit(0x80, registers.h);
}

void GbCpu::bit_7_l()
{
    bit(0x80, registers.l);
}

void GbCpu::bit_7_hlp()
{
    bit(0x80, readByte(registers.hl));
}

void GbCpu::bit_7_a()
{
    bit(0x80, registers.a);
}

void GbCpu::res_0_b()
{
    registers.b &= 0xFE;
}

void GbCpu::res_0_c()
{
    registers.c &= 0xFE;
}

void GbCpu::res_0_d()
{
    registers.d &= 0xFE;
}

void GbCpu::res_0_e()
{
    registers.e &= 0xFE;
}

void GbCpu::res_0_h()
{
    registers.h &= 0xFE;
}

void GbCpu::res_0_l()
{
    registers.l &= 0xFE;
}

void GbCpu::res_0_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0xFE;
    writeByte(registers.hl, value);
}

void GbCpu::res_0_a()
{
    registers.a &= 0xFE;
}

void GbCpu::res_1_b()
{
    registers.b &= 0xFD;
}

void GbCpu::res_1_c()
{
    registers.c &= 0xFD;
}

void GbCpu::res_1_d()
{
    registers.d &= 0xFD;
}

void GbCpu::res_1_e()
{
    registers.e &= 0xFD;
}

void GbCpu::res_1_h()
{
    registers.h &= 0xFD;
}

void GbCpu::res_1_l()
{
    registers.l &= 0xFD;
}

void GbCpu::res_1_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0xFD;
    writeByte(registers.hl, value);
}

void GbCpu::res_1_a()
{
    registers.a &= 0xFD;
}

void GbCpu::res_2_b()
{
    registers.b &= 0xFB;
}

void GbCpu::res_2_c()
{
    registers.c &= 0xFB;
}

void GbCpu::res_2_d()
{
    registers.d &= 0xFB;
}

void GbCpu::res_2_e()
{
    registers.e &= 0xFB;
}

void GbCpu::res_2_h()
{
    registers.h &= 0xFB;
}

void GbCpu::res_2_l()
{
    registers.l &= 0xFB;
}

void GbCpu::res_2_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0xFB;
    writeByte(registers.hl, value);
}

void GbCpu::res_2_a()
{
    registers.a &= 0xFB;
}

void GbCpu::res_3_b()
{
    registers.b &= 0xF7;
}

void GbCpu::res_3_c()
{
    registers.c &= 0xF7;
}

void GbCpu::res_3_d()
{
    registers.d &= 0xF7;
}

void GbCpu::res_3_e()
{
    registers.e &= 0xF7;
}

void GbCpu::res_3_h()
{
    registers.h &= 0xF7;
}

void GbCpu::res_3_l()
{
    registers.l &= 0xF7;
}

void GbCpu::res_3_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0xF7;
    writeByte(registers.hl, value);
}

void GbCpu::res_3_a()
{
    registers.a &= 0xF7;
}

void GbCpu::res_4_b()
{
    registers.b &= 0xEF;
}

void GbCpu::res_4_c()
{
    registers.c &= 0xEF;
}

void GbCpu::res_4_d()
{
    registers.d &= 0xEF;
}

void GbCpu::res_4_e()
{
    registers.e &= 0xEF;
}

void GbCpu::res_4_h()
{
    registers.h &= 0xEF;
}

void GbCpu::res_4_l()
{
    registers.l &= 0xEF;
}

void GbCpu::res_4_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0xEF;
    writeByte(registers.hl, value);
}

void GbCpu::res_4_a()
{
    registers.a &= 0xEF;
}

void GbCpu::res_5_b()
{
    registers.b &= 0xDF;
}

void GbCpu::res_5_c()
{
    registers.c &= 0xDF;
}

void GbCpu::res_5_d()
{
    registers.d &= 0xDF;
}

void GbCpu::res_5_e()
{
    registers.e &= 0xDF;
}

void GbCpu::res_5_h()
{
    registers.h &= 0xDF;
}

void GbCpu::res_5_l()
{
    registers.l &= 0xDF;
}

void GbCpu::res_5_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0xDF;
    writeByte(registers.hl, value);
}

void GbCpu::res_5_a()
{
    registers.a &= 0xDF;
}

void GbCpu::res_6_b()
{
    registers.b &= 0xBF;
}

void GbCpu::res_6_c()
{
    registers.c &= 0xBF;
}

void GbCpu::res_6_d()
{
    registers.d &= 0xBF;
}

void GbCpu::res_6_e()
{
    registers.e &= 0xBF;
}

void GbCpu::res_6_h()
{
    registers.h &= 0xBF;
}

void GbCpu::res_6_l()
{
    registers.l &= 0xBF;
}

void GbCpu::res_6_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0xBF;
    writeByte(registers.hl, value);
}

void GbCpu::res_6_a()
{
    registers.a &= 0xBF;
}

void GbCpu::res_7_b()
{
    registers.b &= 0x7F;
}

void GbCpu::res_7_c()
{
    registers.c &= 0x7F;
}

void GbCpu::res_7_d()
{
    registers.d &= 0x7F;
}

void GbCpu::res_7_e()
{
    registers.e &= 0x7F;
}

void GbCpu::res_7_h()
{
    registers.h &= 0x7F;
}

void GbCpu::res_7_l()
{
    registers.l &= 0x7F;
}

void GbCpu::res_7_hlp()
{
    uint8_t value = readByte(registers.hl);
    value &= 0x7F;
    writeByte(registers.hl, value);
}

void GbCpu::res_7_a()
{
    registers.a &= 0x7F;
}

void GbCpu::set_0_b()
{
    registers.b |= 0x1;
}

void GbCpu::set_0_c()
{
    registers.c |= 0x1;
}

void GbCpu::set_0_d()
{
    registers.d |= 0x1;
}

void GbCpu::set_0_e()
{
    registers.e |= 0x1;
}

void GbCpu::set_0_h()
{
    registers.h |= 0x1;
}

void GbCpu::set_0_l()
{
    registers.l |= 0x1;
}

void GbCpu::set_0_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x1;
    writeByte(registers.hl, value);
}

void GbCpu::set_0_a()
{
    registers.a |= 0x1;
}

void GbCpu::set_1_b()
{
    registers.b |= 0x2;
}

void GbCpu::set_1_c()
{
    registers.c |= 0x2;
}

void GbCpu::set_1_d()
{
    registers.d |= 0x2;
}

void GbCpu::set_1_e()
{
    registers.e |= 0x2;
}

void GbCpu::set_1_h()
{
    registers.h |= 0x2;
}

void GbCpu::set_1_l()
{
    registers.l |= 0x2;
}

void GbCpu::set_1_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x2;
    writeByte(registers.hl, value);
}

void GbCpu::set_1_a()
{
    registers.a |= 0x2;
}

void GbCpu::set_2_b()
{
    registers.b |= 0x4;
}

void GbCpu::set_2_c()
{
    registers.c |= 0x4;
}

void GbCpu::set_2_d()
{
    registers.d |= 0x4;
}

void GbCpu::set_2_e()
{
    registers.e |= 0x4;
}

void GbCpu::set_2_h()
{
    registers.h |= 0x4;
}

void GbCpu::set_2_l()
{
    registers.l |= 0x4;
}

void GbCpu::set_2_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x4;
    writeByte(registers.hl, value);
}

void GbCpu::set_2_a()
{
    registers.a |= 0x4;
}

void GbCpu::set_3_b()
{
    registers.b |= 0x8;
}

void GbCpu::set_3_c()
{
    registers.c |= 0x8;
}

void GbCpu::set_3_d()
{
    registers.d |= 0x8;
}

void GbCpu::set_3_e()
{
    registers.e |= 0x8;
}

void GbCpu::set_3_h()
{
    registers.h |= 0x8;
}

void GbCpu::set_3_l()
{
    registers.l |= 0x8;
}

void GbCpu::set_3_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x8;
    writeByte(registers.hl, value);
}

void GbCpu::set_3_a()
{
    registers.a |= 0x8;
}

void GbCpu::set_4_b()
{
    registers.b |= 0x10;
}

void GbCpu::set_4_c()
{
    registers.c |= 0x10;
}

void GbCpu::set_4_d()
{
    registers.d |= 0x10;
}

void GbCpu::set_4_e()
{
    registers.e |= 0x10;
}

void GbCpu::set_4_h()
{
    registers.h |= 0x10;
}

void GbCpu::set_4_l()
{
    registers.l |= 0x10;
}

void GbCpu::set_4_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x10;
    writeByte(registers.hl, value);
}

void GbCpu::set_4_a()
{
    registers.a |= 0x10;
}

void GbCpu::set_5_b()
{
    registers.b |= 0x20;
}

void GbCpu::set_5_c()
{
    registers.c |= 0x20;
}

void GbCpu::set_5_d()
{
    registers.d |= 0x20;
}

void GbCpu::set_5_e()
{
    registers.e |= 0x20;
}

void GbCpu::set_5_h()
{
    registers.h |= 0x20;
}

void GbCpu::set_5_l()
{
    registers.l |= 0x20;
}

void GbCpu::set_5_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x20;
    writeByte(registers.hl, value);
}

void GbCpu::set_5_a()
{
    registers.a |= 0x20;
}

void GbCpu::set_6_b()
{
    registers.b |= 0x40;
}

void GbCpu::set_6_c()
{
    registers.c |= 0x40;
}

void GbCpu::set_6_d()
{
    registers.d |= 0x40;
}

void GbCpu::set_6_e()
{
    registers.e |= 0x40;
}

void GbCpu::set_6_h()
{
    registers.h |= 0x40;
}

void GbCpu::set_6_l()
{
    registers.l |= 0x40;
}

void GbCpu::set_6_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x40;
    writeByte(registers.hl, value);
}

void GbCpu::set_6_a()
{
    registers.a |= 0x40;
}

void GbCpu::set_7_b()
{
    registers.b |= 0x80;
}

void GbCpu::set_7_c()
{
    registers.c |= 0x80;
}

void GbCpu::set_7_d()
{
    registers.d |= 0x80;
}

void GbCpu::set_7_e()
{
    registers.e |= 0x80;
}

void GbCpu::set_7_h()
{
    registers.h |= 0x80;
}

void GbCpu::set_7_l()
{
    registers.l |= 0x80;
}

void GbCpu::set_7_hlp()
{
    uint8_t value = readByte(registers.hl);
    value |= 0x80;
    writeByte(registers.hl, value);
}

void GbCpu::set_7_a()
{
    registers.a |= 0x80;
}

void GbCpu::rlc(uint8_t& reg)
{
    if (reg & 0x80) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
    reg = (reg << 1) | (reg >> 7);
    setZeroFlag(reg);
}

void GbCpu::rrc(uint8_t& reg)
{
    if (reg & 0x1) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
    setZeroFlag(reg);
    reg = (reg >> 1) | (reg << 7);
}

void GbCpu::rl(uint8_t& reg)
{
    uint8_t carray = registers.carry;
    if (reg & 0x80) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
    reg = (reg << 1) | carray;
    setZeroFlag(reg);
}

void GbCpu::rr(uint8_t& reg)
{
    uint8_t carray = registers.carry;
    if (reg & 0x1) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
    reg = (reg >> 1) | carray << 7;
    setZeroFlag(reg);
}

void GbCpu::sla(uint8_t& reg)
{
    if (reg & 0x80) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
    reg = reg << 1;
    setZeroFlag(reg);
}

void GbCpu::sra(uint8_t& reg)
{
    if (reg & 0x1) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
    reg = (reg & 0x80) | (reg >> 1);
    setZeroFlag(reg);
}

void GbCpu::swap(uint8_t& reg)
{
    registers.carry = 0;
    registers.halfCarry = 0;
    registers.subtraction = 0;
    reg = (reg << 4) | (reg >> 4);
    setZeroFlag(reg);
}

void GbCpu::srl(uint8_t& reg)
{
    if (reg & 0x1) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
    reg = reg >> 1;
    setZeroFlag(reg);
}

void GbCpu::bit(uint8_t bit, uint8_t value)
{
    if (bit & value) {
        registers.zero = 0;
    }
    else {
        registers.zero = 1;
    }
    registers.halfCarry = 0;
    registers.subtraction = 0;
}

}  // namespace qnemu

/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <stdexcept>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/cpu/GbCpu.h"

namespace qnemu
{

void GbCpu::nop()
{
}

void GbCpu::ld_bc_nn()
{
    registers.bc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
}

void GbCpu::ld_bcp_a()
{
    writeByte(registers.bc, registers.a);
}

void GbCpu::inc_bc()
{
    registers.bc++;
}

void GbCpu::inc_b()
{
    inc(registers.b);
}

void GbCpu::dec_b()
{
    dec(registers.b);
}

void GbCpu::ld_b_n()
{
    registers.b = readByte(registers.pc + 1);
}

void GbCpu::rlca()
{
    uint8_t carray = (registers.a & 0x80) >> 7;
    if (carray) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.a = registers.a << 1;
    registers.a |= carray;
    registers.subtraction = 0;
    registers.zero = 0;
    registers.halfCarry = 0;
}

void GbCpu::ld_nnp_sp()
{
    uint16_t address = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
    writeByte(address, registers.sp & 0xFF);
    writeByte(address + 1, registers.sp >> 8);
}

void GbCpu::add_hl_bc()
{
    add_hl(registers.bc);
}

void GbCpu::ld_a_bcp()
{
    registers.a = readByte(registers.bc);
}

void GbCpu::dec_bc()
{
    registers.bc--;
}

void GbCpu::inc_c()
{
    inc(registers.c);
}

void GbCpu::dec_c()
{
    dec(registers.c);
}

void GbCpu::ld_c_n()
{
    registers.c = readByte(registers.pc + 1);
}

void GbCpu::rrca()
{
    uint8_t carray = registers.a & 0x01;
    if (carray) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.a = registers.a >> 1;
    registers.a |= (carray << 7);
    registers.subtraction = 0;
    registers.zero = 0;
    registers.halfCarry = 0;
}

void GbCpu::stop_p()
{
}

void GbCpu::ld_de_nn()
{
    registers.de = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
}

void GbCpu::ld_dep_a()
{
    writeByte(registers.de, registers.a);
}

void GbCpu::inc_de()
{
    registers.de++;
}

void GbCpu::inc_d()
{
    inc(registers.d);
}

void GbCpu::dec_d()
{
    dec(registers.d);
}

void GbCpu::ld_d_n()
{
    registers.d = readByte(registers.pc + 1);
}

void GbCpu::rla()
{
    uint8_t carray = registers.carry;
    if (registers.a & 0x80) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.a = registers.a << 1;
    registers.a |= carray;
    registers.subtraction = 0;
    registers.zero = 0;
    registers.halfCarry = 0;
}

void GbCpu::jr_n()
{
    registers.pc += static_cast<int8_t>(readByte(registers.pc + 1)) + 2;
}

void GbCpu::add_hl_de()
{
    add_hl(registers.de);
}

void GbCpu::ld_a_dep()
{
    registers.a = readByte(registers.de);
}

void GbCpu::dec_de()
{
    registers.de--;
}

void GbCpu::inc_e()
{
    inc(registers.e);
}

void GbCpu::dec_e()
{
    dec(registers.e);
}

void GbCpu::ld_e_n()
{
    registers.e = readByte(registers.pc + 1);
}

void GbCpu::rra()
{
    uint8_t carray = (registers.carry << 7);
    if (registers.a & 0x01) {
        registers.carry = 1;
    }
    else {
        registers.carry = 0;
    }
    registers.a = registers.a >> 1;
    registers.a |= carray;
    registers.subtraction = 0;
    registers.zero = 0;
    registers.halfCarry = 0;
}

void GbCpu::jr_nz_n()
{
    if (!registers.zero) {
       registers.pc += static_cast<int8_t>(readByte(registers.pc + 1)) + 2;
       ticks += 4;
    }
}

void GbCpu::ld_hl_nn()
{
    registers.hl = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
}

void GbCpu::ldi_hlp_a()
{
    writeByte(registers.hl, registers.a);
    registers.hl++;
}

void GbCpu::inc_hl()
{
    registers.hl++;
}

void GbCpu::inc_h()
{
    inc(registers.h);
}

void GbCpu::dec_h()
{
    dec(registers.h);
}

void GbCpu::ld_h_n()
{
    registers.h = readByte(registers.pc + 1);
}

void GbCpu::daa()
{
    if (registers.subtraction) {
        if (registers.halfCarry) {
            registers.a += 0xFA;
        }
        if (registers.carry) {
            registers.a += 0xA0;
        }
    }
    else {
        uint16_t value = registers.a;
        if (registers.halfCarry || (value & 0xF) > 9) {
            value += 0x06;
        }
        if (registers.carry || (value & 0x1F0) > 0x90) {
            value += 0x60;
            registers.carry = 1;
        }
        else {
            registers.carry = 0;
        }
        registers.a = static_cast<uint8_t>(value);
    }
    registers.halfCarry = 0;
    setZeroFlag(registers.a);
}

void GbCpu::jr_z_n()
{
    if (registers.zero) {
        registers.pc += static_cast<int8_t>(readByte(registers.pc + 1)) + 2;
        ticks += 4;
    }
}

void GbCpu::add_hl_hl()
{
    add_hl(registers.hl);
}

void GbCpu::ldi_a_hlp()
{
    registers.a = readByte(registers.hl);
    registers.hl++;
}

void GbCpu::dec_hl()
{
    registers.hl--;
}

void GbCpu::inc_l()
{
    inc(registers.l);
}

void GbCpu::dec_l()
{
    dec(registers.l);
}

void GbCpu::ld_l_n()
{
    registers.l = readByte(registers.pc + 1);
}

void GbCpu::cpl()
{
    registers.a = ~registers.a;
    registers.subtraction = 1;
    registers.halfCarry = 1;
}

void GbCpu::jr_nc_n()
{
    if (!registers.carry) {
        registers.pc += readByte(registers.pc + 1) + 2;
        ticks += 4;
    }
}

void GbCpu::ld_sp_nn()
{
    registers.sp = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
}

void GbCpu::ldd_hlp_a()
{
    writeByte(registers.hl, registers.a);
    registers.hl--;
}

void GbCpu::inc_sp()
{
    registers.sp++;
}

void GbCpu::inc_hlp()
{
    uint8_t value = readByte(registers.hl);
    inc(value);
    writeByte(registers.hl, value);
}

void GbCpu::dec_hlp()
{
    uint8_t value = readByte(registers.hl);
    dec(value);
    writeByte(registers.hl, value);
}

void GbCpu::ld_hlp_n()
{
    uint8_t value = readByte(registers.pc + 1);
    writeByte(registers.hl, value);
}

void GbCpu::scf()
{
    registers.carry = 1;
    registers.subtraction = 0;
    registers.halfCarry = 0;
}

void GbCpu::jr_c_n()
{
    if (registers.carry) {
        registers.pc += readByte(registers.pc + 1) + 2;
        ticks += 4;
    }
}

void GbCpu::add_hl_sp()
{
    add_hl(registers.sp);
}

void GbCpu::ldd_a_hlp()
{
    registers.a = readByte(registers.hl);
    registers.hl--;
}

void GbCpu::dec_sp()
{
    registers.sp--;
}

void GbCpu::inc_a()
{
    inc(registers.a);
}

void GbCpu::dec_a()
{
    dec(registers.a);
}

void GbCpu::ld_a_n()
{
    registers.a = readByte(registers.pc + 1);
}

void GbCpu::ccf()
{
    if (registers.carry) {
        registers.carry = 0;
    }
    else {
        registers.carry = 1;
    }
    registers.subtraction = 0;
    registers.halfCarry = 0;
}

void GbCpu::ld_b_b()
{
    registers.b = registers.b;
}

void GbCpu::ld_b_c()
{
    registers.b = registers.c;
}

void GbCpu::ld_b_d()
{
    registers.b = registers.d;
}

void GbCpu::ld_b_e()
{
    registers.b = registers.e;
}

void GbCpu::ld_b_h()
{
    registers.b = registers.h;
}

void GbCpu::ld_b_l()
{
    registers.b = registers.l;
}

void GbCpu::ld_b_hlp()
{
    registers.b = readByte(registers.hl);
}

void GbCpu::ld_b_a()
{
    registers.b = registers.a;
}

void GbCpu::ld_c_b()
{
    registers.c = registers.b;
}

void GbCpu::ld_c_c()
{
    registers.c = registers.c;
}

void GbCpu::ld_c_d()
{
    registers.c = registers.d;
}

void GbCpu::ld_c_e()
{
    registers.c = registers.e;
}

void GbCpu::ld_c_h()
{
    registers.c = registers.h;
}

void GbCpu::ld_c_l()
{
    registers.c = registers.l;
}

void GbCpu::ld_c_hlp()
{
    registers.c = readByte(registers.hl);
}

void GbCpu::ld_c_a()
{
    registers.c = registers.a;
}

void GbCpu::ld_d_b()
{
    registers.d = registers.b;
}

void GbCpu::ld_d_c()
{
    registers.d = registers.c;
}

void GbCpu::ld_d_d()
{
    registers.d = registers.d;
}

void GbCpu::ld_d_e()
{
    registers.d = registers.e;
}

void GbCpu::ld_d_h()
{
    registers.d = registers.h;
}

void GbCpu::ld_d_l()
{
    registers.d = registers.l;
}

void GbCpu::ld_d_hlp()
{
    registers.d = readByte(registers.hl);
}

void GbCpu::ld_d_a()
{
    registers.d = registers.a;
}

void GbCpu::ld_e_b()
{
    registers.e = registers.b;
}

void GbCpu::ld_e_c()
{
    registers.e = registers.c;
}

void GbCpu::ld_e_d()
{
    registers.e = registers.d;
}

void GbCpu::ld_e_e()
{
    registers.e = registers.e;
}

void GbCpu::ld_e_h()
{
    registers.e = registers.h;
}

void GbCpu::ld_e_l()
{
    registers.e = registers.l;
}

void GbCpu::ld_e_hlp()
{
    registers.e = readByte(registers.hl);
}

void GbCpu::ld_e_a()
{
    registers.e = registers.a;
}

void GbCpu::ld_h_b()
{
    registers.h = registers.b;
}

void GbCpu::ld_h_c()
{
    registers.h = registers.c;
}

void GbCpu::ld_h_d()
{
    registers.h = registers.d;
}

void GbCpu::ld_h_e()
{
    registers.h = registers.e;
}

void GbCpu::ld_h_h()
{
    registers.h = registers.h;
}

void GbCpu::ld_h_l()
{
    registers.h = registers.l;
}

void GbCpu::ld_h_hlp()
{
    registers.h = readByte(registers.hl);
}

void GbCpu::ld_h_a()
{
    registers.h = registers.a;
}

void GbCpu::ld_l_b()
{
    registers.l = registers.b;
}

void GbCpu::ld_l_c()
{
    registers.l = registers.c;
}

void GbCpu::ld_l_d()
{
    registers.l = registers.d;
}

void GbCpu::ld_l_e()
{
    registers.l = registers.e;
}

void GbCpu::ld_l_h()
{
    registers.l = registers.h;
}

void GbCpu::ld_l_l()
{
    registers.l = registers.l;
}

void GbCpu::ld_l_hlp()
{
    registers.l = readByte(registers.hl);
}

void GbCpu::ld_l_a()
{
    registers.l = registers.a;
}

void GbCpu::ld_hlp_b()
{
    writeByte(registers.hl, registers.b);
}

void GbCpu::ld_hlp_c()
{
    writeByte(registers.hl, registers.c);
}

void GbCpu::ld_hlp_d()
{
    writeByte(registers.hl, registers.d);
}

void GbCpu::ld_hlp_e()
{
    writeByte(registers.hl, registers.e);
}

void GbCpu::ld_hlp_h()
{
    writeByte(registers.hl, registers.h);
}

void GbCpu::ld_hlp_l()
{
    writeByte(registers.hl, registers.l);
}

void GbCpu::halt()
{
    if(GbDeviceInterface::interruptMasterEnabled) {
    }
    else {
    }
}

void GbCpu::ld_hlp_a()
{
    writeByte(registers.hl, registers.a);
}

void GbCpu::ld_a_b()
{
    registers.a = registers.b;
}

void GbCpu::ld_a_c()
{
    registers.a = registers.c;
}

void GbCpu::ld_a_d()
{
    registers.a = registers.d;
}

void GbCpu::ld_a_e()
{
    registers.a = registers.e;
}

void GbCpu::ld_a_h()
{
    registers.a = registers.h;
}

void GbCpu::ld_a_l()
{
    registers.a = registers.l;
}

void GbCpu::ld_a_hlp()
{
    registers.a = readByte(registers.hl);
}

void GbCpu::ld_a_a()
{
    registers.a = registers.a;
}

void GbCpu::add_a_b()
{
    add_a(registers.b);
}

void GbCpu::add_a_c()
{
    add_a(registers.c);
}

void GbCpu::add_a_d()
{
    add_a(registers.d);
}

void GbCpu::add_a_e()
{
    add_a(registers.e);
}

void GbCpu::add_a_h()
{
    add_a(registers.h);
}

void GbCpu::add_a_l()
{
    add_a(registers.l);
}

void GbCpu::add_a_hlp()
{
    uint8_t value = readByte(registers.hl);
    add_a(value);
}

void GbCpu::add_a_a()
{
    add_a(registers.a);
}

void GbCpu::adc_b()
{
    adc(registers.b);
}

void GbCpu::adc_c()
{
    adc(registers.c);
}

void GbCpu::adc_d()
{
    adc(registers.d);
}

void GbCpu::adc_e()
{
    adc(registers.e);
}

void GbCpu::adc_h()
{
    adc(registers.h);
}

void GbCpu::adc_l()
{
    adc(registers.l);
}

void GbCpu::adc_hlp()
{
    adc(readByte(registers.hl));
}

void GbCpu::adc_a()
{
    adc(registers.a);
}

void GbCpu::sub_b()
{
    sub(registers.b);
}

void GbCpu::sub_c()
{
    sub(registers.c);
}

void GbCpu::sub_d()
{
    sub(registers.d);
}

void GbCpu::sub_e()
{
    sub(registers.e);
}

void GbCpu::sub_h()
{
    sub(registers.h);
}

void GbCpu::sub_l()
{
    sub(registers.l);
}

void GbCpu::sub_hlp()
{
    sub(readByte(registers.hl));
}

void GbCpu::sub_a()
{
    sub(registers.a);
}

void GbCpu::sbc_b()
{
    sbc(registers.b);
}

void GbCpu::sbc_c()
{
    sbc(registers.c);
}

void GbCpu::sbc_d()
{
    sbc(registers.d);
}

void GbCpu::sbc_e()
{
    sbc(registers.e);
}

void GbCpu::sbc_h()
{
    sbc(registers.h);
}

void GbCpu::sbc_l()
{
    sbc(registers.l);
}

void GbCpu::sbc_hlp()
{
    sbc(readByte(registers.hl));
}

void GbCpu::sbc_a()
{
    sbc(registers.a);
}

void GbCpu::and_b()
{
    and_(registers.b);
}

void GbCpu::and_c()
{
    and_(registers.c);
}

void GbCpu::and_d()
{
    and_(registers.d);
}

void GbCpu::and_e()
{
    and_(registers.e);
}

void GbCpu::and_h()
{
    and_(registers.h);
}

void GbCpu::and_l()
{
    and_(registers.l);
}

void GbCpu::and_hlp()
{
    and_(readByte(registers.hl));
}

void GbCpu::and_a()
{
    and_(registers.a);
}

void GbCpu::xor_b()
{
    xor_(registers.b);
}

void GbCpu::xor_c()
{
    xor_(registers.c);
}

void GbCpu::xor_d()
{
    xor_(registers.d);
}

void GbCpu::xor_e()
{
    xor_(registers.e);
}

void GbCpu::xor_h()
{
    xor_(registers.h);
}

void GbCpu::xor_l()
{
    xor_(registers.l);
}

void GbCpu::xor_hlp()
{
    xor_(readByte(registers.hl));
}

void GbCpu::xor_a()
{
    xor_(registers.a);
}

void GbCpu::or_b()
{
    or_(registers.b);
}

void GbCpu::or_c()
{
    or_(registers.c);
}

void GbCpu::or_d()
{
    or_(registers.d);
}

void GbCpu::or_e()
{
    or_(registers.e);
}

void GbCpu::or_h()
{
    or_(registers.h);
}

void GbCpu::or_l()
{
    or_(registers.l);
}

void GbCpu::or_hlp()
{
    or_(readByte(registers.hl));
}

void GbCpu::or_a()
{
    or_(registers.a);
}

void GbCpu::cp_b()
{
    cp(registers.b);
}

void GbCpu::cp_c()
{
    cp(registers.c);
}

void GbCpu::cp_d()
{
    cp(registers.d);
}

void GbCpu::cp_e()
{
    cp(registers.e);
}

void GbCpu::cp_h()
{
    cp(registers.h);
}

void GbCpu::cp_l()
{
    cp(registers.l);
}

void GbCpu::cp_hlp()
{
    cp(readByte(registers.hl));
}

void GbCpu::cp_a()
{
    cp(registers.a);
}

void GbCpu::ret_nz()
{
    if (!registers.zero) {
        registers.pc = pop();
        ticks += 12;
    }
}

void GbCpu::pop_bc()
{
    registers.bc = pop();
}

void GbCpu::jp_nz_nn()
{
    if (!registers.zero) {
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 4;
    }
}

void GbCpu::jp_nn()
{
    registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
}

void GbCpu::call_nz_nn()
{
    if (!registers.zero) {
        push(registers.pc + 3);
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 12;
    }
}

void GbCpu::push_bc()
{
    push(registers.bc);
}

void GbCpu::add_a_n()
{
    add_a(readByte(registers.pc + 1));
}

void GbCpu::rst_0()
{
    push(registers.pc + 1);
    registers.pc = 0;
}

void GbCpu::ret_z()
{
    if (registers.zero) {
        registers.pc = pop();
        ticks += 12;
    }
}

void GbCpu::ret()
{
    registers.pc = pop();
}

void GbCpu::jp_z_nn()
{
    if (registers.zero) {
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 4;
    }
}

void GbCpu::cb_n()
{
    auto prefixedInstruction = prefixedInstructions.at(readByte(registers.pc + 1));
    fprintf(fp, prefixedInstruction.disassembly, readByte(registers.pc + 2));
    ticks = prefixedInstruction.ticks;
    prefixedInstruction.execute();
    registers.pc += prefixedInstruction.length;
}

void GbCpu::call_z_nn()
{
    if (registers.zero) {
        push(registers.pc + 3);
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 12;
    }
}

void GbCpu::call_nn()
{
    push(registers.pc + 3);
    registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
}

void GbCpu::adc_n()
{
    adc(readByte(registers.pc + 1));
}

void GbCpu::rst_08()
{
    push(registers.pc + 1);
    registers.pc = 0x8;
}

void GbCpu::ret_nc()
{
    if (!registers.carry) {
        registers.pc = pop();
        ticks += 12;
    }
}

void GbCpu::pop_de()
{
    registers.de = pop();
}

void GbCpu::jp_nc_nn()
{
    if (!registers.carry) {
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 4;
    }
}

void GbCpu::call_nc_nn()
{
    if (!registers.carry) {
        push(registers.pc + 3);
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 12;
    }
}

void GbCpu::push_de()
{
    push(registers.de);
}

void GbCpu::sub_n()
{
    sub(readByte(registers.pc + 1));
}

void GbCpu::rst_10()
{
    push(registers.pc + 1);
    registers.pc = 0x10;
}

void GbCpu::ret_c()
{
    if (registers.carry) {
        registers.pc = pop();
        ticks += 12;
    }
}

void GbCpu::reti()
{
    enableInterruptFlag = true;
    registers.pc = pop();
}

void GbCpu::jp_c_nn()
{
    if (registers.carry) {
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 4;
    }
}

void GbCpu::call_c_nn()
{
    if (registers.carry) {
        push(registers.pc + 3);
        registers.pc = readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8);
        ticks += 12;
    }
}

void GbCpu::sbc_n()
{
    sbc(readByte(registers.pc + 1));
}

void GbCpu::rst_18()
{
    push(registers.pc + 1);
    registers.pc = 0x18;
}

void GbCpu::ld_ff_n_ap()
{
    writeByte(0xFF00 + readByte(registers.pc + 1), registers.a);
}

void GbCpu::pop_hl()
{
    registers.hl = pop();
}

void GbCpu::ld_ff_c_a()
{
    writeByte(0xFF00 + registers.c, registers.a);
}

void GbCpu::push_hl()
{
    push(registers.hl);
}

void GbCpu::and_n()
{
    and_(readByte(registers.pc + 1));
}

void GbCpu::rst_20()
{
    push(registers.pc + 1);
    registers.pc = 0x20;
}

void GbCpu::add_sp_n()
{
    uint16_t value = readByte(registers.pc + 1);
    setCarryFlag(registers.sp, value, true);
    setHalfCarryFlag(((registers.sp >> 8) & 0x0F), ((value >> 8) & 0x0F), true);
    registers.sp += value;
    registers.subtraction = 0;
    registers.zero = 0;
}

void GbCpu::jp_hl()
{
    registers.pc = registers.hl;
}

void GbCpu::ld_nnp_a()
{
    writeByte(readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8), registers.a);
}

void GbCpu::xor_n()
{
    xor_(readByte(registers.pc + 1));
}

void GbCpu::rst_28()
{
    push(registers.pc + 1);
    registers.pc = 0x28;
}

void GbCpu::ld_ff_ap_n()
{
    registers.a = readByte(0xFF00 + readByte(registers.pc + 1));
}

void GbCpu::pop_af()
{
    registers.af = pop();
    registers.fReserved = 0;
}

void GbCpu::ld_a_ff_c()
{
    registers.a = readByte(0xFF00 + registers.c);
}

void GbCpu::di_inst()
{
    GbDeviceInterface::interruptMasterEnabled = false;
}

void GbCpu::push_af()
{
    push(registers.af);
}

void GbCpu::or_n()
{
    or_(readByte(registers.pc + 1));
}

void GbCpu::rst_30()
{
    push(registers.pc + 1);
    registers.pc = 0x30;
}

void GbCpu::ld_hl_sp_n()
{
    uint16_t value = readByte(registers.pc + 1);
    setCarryFlag(static_cast<uint16_t>(registers.sp + static_cast<int8_t>(value)), 0, true);
    setHalfCarryFlag(static_cast<uint8_t>(registers.sp), static_cast<uint8_t>(value), true);
    registers.subtraction = 0;
    registers.zero = 0;
    registers.hl = value;
}

void GbCpu::ld_sp_hl()
{
    registers.sp = registers.hl;
}

void GbCpu::ld_a_nnp()
{
    registers.a = readByte(readByte(registers.pc + 1) | (static_cast<uint16_t>(readByte(registers.pc + 2)) << 8));
}

void GbCpu::ei()
{
    enableInterruptFlag = true;
}

void GbCpu::cp_n()
{
    cp(readByte(registers.pc + 1));
}

void GbCpu::rst_38()
{
    push(registers.pc + 1);
    registers.pc = 0x38;
}

void GbCpu::undefined()
{
    if (fp) {
        std::fclose(fp);
    }
    throw std::runtime_error("Invalid instruction!");
}

void GbCpu::inc(uint8_t &reg)
{
    setHalfCarryFlag(reg, 1, true);
    reg++;
    setZeroFlag(reg);
    registers.subtraction = 0;
}

void GbCpu::dec(uint8_t &reg)
{
    setHalfCarryFlag(reg, 1, false);
    reg--;
    setZeroFlag(reg);
    registers.subtraction = 1;
}

void GbCpu::add_hl(uint16_t &reg)
{
    setCarryFlag(registers.hl, reg, true);
    setHalfCarryFlag(((registers.hl >> 8) & 0x0F), ((reg >> 8) & 0x0F), true);
    registers.hl += reg;
    registers.subtraction = 0;
}

void GbCpu::add_a(uint8_t value)
{
    setCarryFlag(registers.a, value, true);
    setHalfCarryFlag(registers.a, value, true);
    registers.a += value;
    setZeroFlag(registers.a);
    registers.subtraction = 0;
}

void GbCpu::adc(uint8_t reg)
{
    uint8_t value = reg + (registers.carry ? 1 : 0);
    setCarryFlag(registers.a, value, true);
    setHalfCarryFlag(registers.a, value, true);
    registers.a += value;
    setZeroFlag(registers.a);
    registers.subtraction = 0;
}

void GbCpu::sub(uint8_t value)
{
    setCarryFlag(registers.a, value, false);
    setHalfCarryFlag(registers.a, value, false);
    registers.a -= value;
    setZeroFlag(registers.a);
    registers.subtraction = 1;
}

void GbCpu::sbc(uint8_t reg)
{
    uint8_t value = reg + (registers.carry ? 1 : 0);
    setCarryFlag(registers.a, value, false);
    setHalfCarryFlag(registers.a, value, false);
    registers.a -= value;
    setZeroFlag(registers.a);
    registers.subtraction = 1;
}

void GbCpu::and_(uint8_t value)
{
    registers.a &= value;
    setZeroFlag(registers.a);
    registers.carry = 0;
    registers.halfCarry = 1;
    registers.subtraction = 0;
}

void GbCpu::xor_(uint8_t value)
{
    registers.a ^= value;
    setZeroFlag(registers.a);
    registers.carry = 0;
    registers.halfCarry = 0;
    registers.subtraction = 0;
}

void GbCpu::or_(uint8_t value)
{
    registers.a |= value;
    setZeroFlag(registers.a);
    registers.carry = 0;
    registers.halfCarry = 0;
    registers.subtraction = 0;
}

void GbCpu::cp(uint8_t value)
{
    setCarryFlag(registers.a, value, false);
    setHalfCarryFlag(registers.a, value, false);
    setZeroFlag(registers.a - value);
    registers.subtraction = 1;
}

}  // namespace qnemu

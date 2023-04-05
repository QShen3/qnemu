/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/cpu/Instruction.h"
#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/mmu/GbMmuInterface.h"

namespace qnemu
{

class GbGpu;

class GbCpu : public GbCpuInterface
{
public:
    GbCpu();
    ~GbCpu();

    void start() override;
    void stop() override;
    void reset() override;

    bool isInHaltMode() const override;
    void exitHaltMode() override;
    bool isInStopMode() const override;
    void exitStopMode() override;
    void jumpToAddress(uint16_t address) override;
    void cancelInterrupt() override;
    void addMmu(std::unique_ptr<GbMmuInterface> mmu) override;
    void addDisplay(std::shared_ptr<DisplayInterface> display) override;

private:
    friend GbGpu;
    void exec();
    void step();
    uint8_t readByte(uint16_t address) const override;
    void writeByte(uint16_t address, uint8_t value) override;

    struct {
        union {
            struct {
                struct {
                    uint8_t fReserved : 4;
                    uint8_t carry : 1;
                    uint8_t halfCarry : 1;
                    uint8_t subtraction  : 1;
                    uint8_t zero : 1;
                };
                uint8_t a;
            };
            uint16_t af;
        };
        union {
            struct {
                uint8_t c;
                uint8_t b;
            };
            uint16_t bc;
        };
        union {
            struct {
                uint8_t e;
                uint8_t d;
            };
            uint16_t de;
        };
        union {
            struct {
                uint8_t l;
                uint8_t h;
            };
            uint16_t hl;
        };
        uint16_t sp;
        uint16_t pc;
    } registers;

    std::unique_ptr<GbMmuInterface> mmu;
    std::shared_ptr<DisplayInterface> display;
    bool enableInterruptFlag;
    std::atomic_bool halt_mode;
    std::atomic_bool stop_mode;
    std::atomic_bool started;
    uint8_t ticks;
    std::thread work;
    const std::array<Instruction, 256> instructions;
    const std::array<Instruction, 256> prefixedInstructions;

    // Instructions
    void nop();       // 0x00
    void ld_bc_nn();  // 0x01
    void ld_bcp_a();  // 0x02
    void inc_bc();    // 0x03
    void inc_b();     // 0x04
    void dec_b();     // 0x05
    void ld_b_n();    // 0x06
    void rlca();      // 0x07
    void ld_nnp_sp();  // 0x08
    void add_hl_bc();  // 0x09
    void ld_a_bcp();  // 0x0A
    void dec_bc();    // 0x0B
    void inc_c();     // 0x0C
    void dec_c();     // 0x0D
    void ld_c_n();    // 0x0E
    void rrca();      // 0x0F

    void stop_p();    // 0x10
    void ld_de_nn();  // 0x11
    void ld_dep_a();  // 0x12
    void inc_de();    // 0x13
    void inc_d();     // 0x14
    void dec_d();     // 0x15
    void ld_d_n();    // 0x16
    void rla();       // 0x17
    void jr_n();      // 0x18
    void add_hl_de();  // 0x19
    void ld_a_dep();  // 0x1A
    void dec_de();    // 0x1B
    void inc_e();     // 0x1C
    void dec_e();     // 0x1D
    void ld_e_n();    // 0x1E
    void rra();       // 0x1F

    void jr_nz_n();    // 0x20
    void ld_hl_nn();  // 0x21
    void ldi_hlp_a();  // 0x22
    void inc_hl();    // 0x23
    void inc_h();     // 0x24
    void dec_h();     // 0x25
    void ld_h_n();    // 0x26
    void daa();       // 0x27
    void jr_z_n();      // 0x28
    void add_hl_hl();  // 0x29
    void ldi_a_hlp();  // 0x2A
    void dec_hl();    // 0x2B
    void inc_l();     // 0x2C
    void dec_l();     // 0x2D
    void ld_l_n();    // 0x2E
    void cpl();       // 0x2F

    void jr_nc_n();    // 0x30
    void ld_sp_nn();  // 0x31
    void ldd_hlp_a();  // 0x32
    void inc_sp();    // 0x33
    void inc_hlp();     // 0x34
    void dec_hlp();     // 0x35
    void ld_hlp_n();    // 0x36
    void scf();       // 0x37
    void jr_c_n();      // 0x38
    void add_hl_sp();  // 0x39
    void ldd_a_hlp();  // 0x3A
    void dec_sp();    // 0x3B
    void inc_a();     // 0x3C
    void dec_a();     // 0x3D
    void ld_a_n();    // 0x3E
    void ccf();       // 0x3F

    void ld_b_b();    // 0x40
    void ld_b_c();  // 0x41
    void ld_b_d();  // 0x42
    void ld_b_e();    // 0x43
    void ld_b_h();     // 0x44
    void ld_b_l();     // 0x45
    void ld_b_hlp();    // 0x46
    void ld_b_a();       // 0x47
    void ld_c_b();      // 0x48
    void ld_c_c();  // 0x49
    void ld_c_d();  // 0x4A
    void ld_c_e();    // 0x4B
    void ld_c_h();     // 0x4C
    void ld_c_l();     // 0x4D
    void ld_c_hlp();    // 0x4E
    void ld_c_a();       // 0x4F

    void ld_d_b();    // 0x50
    void ld_d_c();  // 0x51
    void ld_d_d();  // 0x52
    void ld_d_e();    // 0x53
    void ld_d_h();     // 0x54
    void ld_d_l();     // 0x55
    void ld_d_hlp();    // 0x56
    void ld_d_a();       // 0x57
    void ld_e_b();      // 0x58
    void ld_e_c();  // 0x59
    void ld_e_d();  // 0x5A
    void ld_e_e();    // 0x5B
    void ld_e_h();     // 0x5C
    void ld_e_l();     // 0x5D
    void ld_e_hlp();    // 0x5E
    void ld_e_a();       // 0x5F

    void ld_h_b();    // 0x60
    void ld_h_c();  // 0x61
    void ld_h_d();  // 0x62
    void ld_h_e();    // 0x63
    void ld_h_h();     // 0x64
    void ld_h_l();     // 0x65
    void ld_h_hlp();    // 0x66
    void ld_h_a();       // 0x67
    void ld_l_b();      // 0x68
    void ld_l_c();  // 0x69
    void ld_l_d();  // 0x6A
    void ld_l_e();    // 0x6B
    void ld_l_h();     // 0x6C
    void ld_l_l();     // 0x6D
    void ld_l_hlp();    // 0x6E
    void ld_l_a();       // 0x6F

    void ld_hlp_b();    // 0x70
    void ld_hlp_c();  // 0x71
    void ld_hlp_d();  // 0x72
    void ld_hlp_e();    // 0x73
    void ld_hlp_h();     // 0x74
    void ld_hlp_l();     // 0x75
    void halt();    // 0x76
    void ld_hlp_a();       // 0x77
    void ld_a_b();      // 0x78
    void ld_a_c();  // 0x79
    void ld_a_d();  // 0x7A
    void ld_a_e();    // 0x7B
    void ld_a_h();     // 0x7C
    void ld_a_l();     // 0x7D
    void ld_a_hlp();    // 0x7E
    void ld_a_a();       // 0x7F

    void add_a_b();    // 0x80
    void add_a_c();  // 0x81
    void add_a_d();  // 0x82
    void add_a_e();    // 0x83
    void add_a_h();     // 0x84
    void add_a_l();     // 0x85
    void add_a_hlp();    // 0x86
    void add_a_a();       // 0x87
    void adc_b();      // 0x88
    void adc_c();  // 0x89
    void adc_d();  // 0x8A
    void adc_e();    // 0x8B
    void adc_h();     // 0x8C
    void adc_l();     // 0x8D
    void adc_hlp();    // 0x8E
    void adc_a();       // 0x8F

    void sub_b();    // 0x90
    void sub_c();  // 0x91
    void sub_d();  // 0x92
    void sub_e();    // 0x93
    void sub_h();     // 0x94
    void sub_l();     // 0x95
    void sub_hlp();    // 0x96
    void sub_a();       // 0x97
    void sbc_b();      // 0x98
    void sbc_c();  // 0x99
    void sbc_d();  // 0x9A
    void sbc_e();    // 0x9B
    void sbc_h();     // 0x9C
    void sbc_l();     // 0x9D
    void sbc_hlp();    // 0x9E
    void sbc_a();       // 0x9F

    void and_b();    // 0xA0
    void and_c();  // 0xA1
    void and_d();  // 0xA2
    void and_e();    // 0xA3
    void and_h();     // 0xA4
    void and_l();     // 0xA5
    void and_hlp();    // 0xA6
    void and_a();       // 0xA7
    void xor_b();      // 0xA8
    void xor_c();  // 0xA9
    void xor_d();  // 0xAA
    void xor_e();    // 0xAB
    void xor_h();     // 0xAC
    void xor_l();     // 0xAD
    void xor_hlp();    // 0xAE
    void xor_a();       // 0xAF

    void or_b();    // 0xB0
    void or_c();  // 0xB1
    void or_d();  // 0xB2
    void or_e();    // 0xB3
    void or_h();     // 0xB4
    void or_l();     // 0xB5
    void or_hlp();    // 0xB6
    void or_a();       // 0xB7
    void cp_b();      // 0xB8
    void cp_c();  // 0xB9
    void cp_d();  // 0xBA
    void cp_e();    // 0xBB
    void cp_h();     // 0xBC
    void cp_l();     // 0xBD
    void cp_hlp();    // 0xBE
    void cp_a();       // 0xBF

    void ret_nz();    // 0xC0
    void pop_bc();  // 0xC1
    void jp_nz_nn();  // 0xC2
    void jp_nn();    // 0xC3
    void call_nz_nn();     // 0xC4
    void push_bc();     // 0xC5
    void add_a_n();    // 0xC6
    void rst_0();       // 0xC7
    void ret_z();      // 0xC8
    void ret();  // 0xC9
    void jp_z_nn();  // 0xCA
    void cb_n();    // 0xCB
    void call_z_nn();     // 0xCC
    void call_nn();     // 0xCD
    void adc_n();    // 0xCE
    void rst_08();       // 0xCF

    void ret_nc();    // 0xD0
    void pop_de();  // 0xD1
    void jp_nc_nn();  // 0xD2
    void call_nc_nn();     // 0xD4
    void push_de();     // 0xD5
    void sub_n();    // 0xD6
    void rst_10();       // 0xD7
    void ret_c();      // 0xD8
    void reti();  // 0xD9
    void jp_c_nn();  // 0xDA
    void call_c_nn();     // 0xDC
    void sbc_n();    // 0xDE
    void rst_18();       // 0xDF

    void ld_ff_n_ap();    // 0xE0
    void pop_hl();  // 0xE1
    void ld_ff_c_a();  // 0xE2
    void push_hl();     // 0xE5
    void and_n();    // 0xE6
    void rst_20();       // 0xE7
    void add_sp_n();      // 0xE8
    void jp_hl();  // 0xE9
    void ld_nnp_a();  // 0xEA
    void xor_n();    // 0xEE
    void rst_28();       // 0xEF

    void ld_ff_ap_n();    // 0xF0
    void pop_af();  // 0xF1
    void ld_a_ff_c();  // 0xF2
    void di_inst();    // 0xF3
    void push_af();     // 0xF5
    void or_n();    // 0xF6
    void rst_30();       // 0xF7
    void ld_hl_sp_n();      // 0xF8
    void ld_sp_hl();  // 0xF9
    void ld_a_nnp();  // 0xFA
    void ei();    // 0xFB
    void cp_n();    // 0xFE
    void rst_38();       // 0xFF

    // Prefixed instructions
    void rlc_b();       // 0x00
    void rlc_c();  // 0x01
    void rlc_d();  // 0x02
    void rlc_e();    // 0x03
    void rlc_h();     // 0x04
    void rlc_l();     // 0x05
    void rlc_hlp();    // 0x06
    void rlc_a();      // 0x07
    void rrc_b();  // 0x08
    void rrc_c();  // 0x09
    void rrc_d();  // 0x0A
    void rrc_e();    // 0x0B
    void rrc_h();     // 0x0C
    void rrc_l();     // 0x0D
    void rrc_hlp();    // 0x0E
    void rrc_a();      // 0x0F

    void rl_b();       // 0x10
    void rl_c();  // 0x11
    void rl_d();  // 0x12
    void rl_e();    // 0x13
    void rl_h();     // 0x14
    void rl_l();     // 0x15
    void rl_hlp();    // 0x16
    void rl_a();      // 0x17
    void rr_b();  // 0x18
    void rr_c();  // 0x19
    void rr_d();  // 0x1A
    void rr_e();    // 0x1B
    void rr_h();     // 0x1C
    void rr_l();     // 0x1D
    void rr_hlp();    // 0x1E
    void rr_a();      // 0x1F

    void sla_b();       // 0x20
    void sla_c();  // 0x21
    void sla_d();  // 0x22
    void sla_e();    // 0x23
    void sla_h();     // 0x24
    void sla_l();     // 0x25
    void sla_hlp();    // 0x26
    void sla_a();      // 0x27
    void sra_b();  // 0x28
    void sra_c();  // 0x29
    void sra_d();  // 0x2A
    void sra_e();    // 0x2B
    void sra_h();     // 0x2C
    void sra_l();     // 0x2D
    void sra_hlp();    // 0x2E
    void sra_a();      // 0x2F

    void swap_b();       // 0x30
    void swap_c();  // 0x31
    void swap_d();  // 0x32
    void swap_e();    // 0x33
    void swap_h();     // 0x34
    void swap_l();     // 0x35
    void swap_hlp();    // 0x36
    void swap_a();      // 0x37
    void srl_b();  // 0x38
    void srl_c();  // 0x39
    void srl_d();  // 0x3A
    void srl_e();    // 0x3B
    void srl_h();     // 0x3C
    void srl_l();     // 0x3D
    void srl_hlp();    // 0x3E
    void srl_a();      // 0x3F

    void bit_0_b();       // 0x40
    void bit_0_c();  // 0x41
    void bit_0_d();  // 0x42
    void bit_0_e();    // 0x43
    void bit_0_h();     // 0x44
    void bit_0_l();     // 0x45
    void bit_0_hlp();    // 0x46
    void bit_0_a();      // 0x47
    void bit_1_b();  // 0x48
    void bit_1_c();  // 0x49
    void bit_1_d();  // 0x4A
    void bit_1_e();    // 0x4B
    void bit_1_h();     // 0x4C
    void bit_1_l();     // 0x4D
    void bit_1_hlp();    // 0x4E
    void bit_1_a();      // 0x4F

    void bit_2_b();       // 0x50
    void bit_2_c();  // 0x51
    void bit_2_d();  // 0x52
    void bit_2_e();    // 0x53
    void bit_2_h();     // 0x54
    void bit_2_l();     // 0x55
    void bit_2_hlp();    // 0x56
    void bit_2_a();      // 0x57
    void bit_3_b();  // 0x58
    void bit_3_c();  // 0x59
    void bit_3_d();  // 0x5A
    void bit_3_e();    // 0x5B
    void bit_3_h();     // 0x5C
    void bit_3_l();     // 0x5D
    void bit_3_hlp();    // 0x5E
    void bit_3_a();      // 0x5F

    void bit_4_b();       // 0x60
    void bit_4_c();  // 0x61
    void bit_4_d();  // 0x62
    void bit_4_e();    // 0x63
    void bit_4_h();     // 0x64
    void bit_4_l();     // 0x65
    void bit_4_hlp();    // 0x66
    void bit_4_a();      // 0x67
    void bit_5_b();  // 0x68
    void bit_5_c();  // 0x69
    void bit_5_d();  // 0x6A
    void bit_5_e();    // 0x6B
    void bit_5_h();     // 0x6C
    void bit_5_l();     // 0x6D
    void bit_5_hlp();    // 0x6E
    void bit_5_a();      // 0x6F

    void bit_6_b();       // 0x70
    void bit_6_c();  // 0x71
    void bit_6_d();  // 0x72
    void bit_6_e();    // 0x73
    void bit_6_h();     // 0x74
    void bit_6_l();     // 0x75
    void bit_6_hlp();    // 0x76
    void bit_6_a();      // 0x77
    void bit_7_b();  // 0x78
    void bit_7_c();  // 0x79
    void bit_7_d();  // 0x7A
    void bit_7_e();    // 0x7B
    void bit_7_h();     // 0x7C
    void bit_7_l();     // 0x7D
    void bit_7_hlp();    // 0x7E
    void bit_7_a();      // 0x7F

    void res_0_b();       // 0x80
    void res_0_c();  // 0x81
    void res_0_d();  // 0x82
    void res_0_e();    // 0x83
    void res_0_h();     // 0x84
    void res_0_l();     // 0x85
    void res_0_hlp();    // 0x86
    void res_0_a();      // 0x87
    void res_1_b();  // 0x88
    void res_1_c();  // 0x89
    void res_1_d();  // 0x8A
    void res_1_e();    // 0x8B
    void res_1_h();     // 0x8C
    void res_1_l();     // 0x8D
    void res_1_hlp();    // 0x8E
    void res_1_a();      // 0x8F

    void res_2_b();       // 0x90
    void res_2_c();  // 0x91
    void res_2_d();  // 0x92
    void res_2_e();    // 0x93
    void res_2_h();     // 0x94
    void res_2_l();     // 0x95
    void res_2_hlp();    // 0x96
    void res_2_a();      // 0x97
    void res_3_b();  // 0x98
    void res_3_c();  // 0x99
    void res_3_d();  // 0x9A
    void res_3_e();    // 0x9B
    void res_3_h();     // 0x9C
    void res_3_l();     // 0x9D
    void res_3_hlp();    // 0x9E
    void res_3_a();      // 0x9F

    void res_4_b();       // 0xA0
    void res_4_c();  // 0xA1
    void res_4_d();  // 0xA2
    void res_4_e();    // 0xA3
    void res_4_h();     // 0xA4
    void res_4_l();     // 0xA5
    void res_4_hlp();    // 0xA6
    void res_4_a();      // 0xA7
    void res_5_b();  // 0xA8
    void res_5_c();  // 0xA9
    void res_5_d();  // 0xAA
    void res_5_e();    // 0xAB
    void res_5_h();     // 0xAC
    void res_5_l();     // 0xAD
    void res_5_hlp();    // 0xAE
    void res_5_a();      // 0xAF

    void res_6_b();       // 0xB0
    void res_6_c();  // 0xB1
    void res_6_d();  // 0xB2
    void res_6_e();    // 0xB3
    void res_6_h();     // 0xB4
    void res_6_l();     // 0xB5
    void res_6_hlp();    // 0xB6
    void res_6_a();      // 0xB7
    void res_7_b();  // 0xB8
    void res_7_c();  // 0xB9
    void res_7_d();  // 0xBA
    void res_7_e();    // 0xBB
    void res_7_h();     // 0xBC
    void res_7_l();     // 0xBD
    void res_7_hlp();    // 0xBE
    void res_7_a();      // 0xBF

    void set_0_b();       // 0xC0
    void set_0_c();  // 0xC1
    void set_0_d();  // 0xC2
    void set_0_e();    // 0xC3
    void set_0_h();     // 0xC4
    void set_0_l();     // 0xC5
    void set_0_hlp();    // 0xC6
    void set_0_a();      // 0xC7
    void set_1_b();  // 0xC8
    void set_1_c();  // 0xC9
    void set_1_d();  // 0xCA
    void set_1_e();    // 0xCB
    void set_1_h();     // 0xCC
    void set_1_l();     // 0xCD
    void set_1_hlp();    // 0xCE
    void set_1_a();      // 0xCF

    void set_2_b();       // 0xD0
    void set_2_c();  // 0xD1
    void set_2_d();  // 0xD2
    void set_2_e();    // 0xD3
    void set_2_h();     // 0xD4
    void set_2_l();     // 0xD5
    void set_2_hlp();    // 0xD6
    void set_2_a();      // 0xD7
    void set_3_b();  // 0xD8
    void set_3_c();  // 0xD9
    void set_3_d();  // 0xDA
    void set_3_e();    // 0xDB
    void set_3_h();     // 0xDC
    void set_3_l();     // 0xDD
    void set_3_hlp();    // 0xDE
    void set_3_a();      // 0xDF

    void set_4_b();       // 0xE0
    void set_4_c();  // 0xE1
    void set_4_d();  // 0xE2
    void set_4_e();    // 0xE3
    void set_4_h();     // 0xE4
    void set_4_l();     // 0xE5
    void set_4_hlp();    // 0xE6
    void set_4_a();      // 0xE7
    void set_5_b();  // 0xE8
    void set_5_c();  // 0xE9
    void set_5_d();  // 0xEA
    void set_5_e();    // 0xEB
    void set_5_h();     // 0xEC
    void set_5_l();     // 0xED
    void set_5_hlp();    // 0xEE
    void set_5_a();      // 0xEF

    void set_6_b();       // 0xF0
    void set_6_c();  // 0xF1
    void set_6_d();  // 0xF2
    void set_6_e();    // 0xF3
    void set_6_h();     // 0xF4
    void set_6_l();     // 0xF5
    void set_6_hlp();    // 0xF6
    void set_6_a();      // 0xF7
    void set_7_b();  // 0xF8
    void set_7_c();  // 0xF9
    void set_7_d();  // 0xFA
    void set_7_e();    // 0xFB
    void set_7_h();     // 0xFC
    void set_7_l();     // 0xFD
    void set_7_hlp();    // 0xFE
    void set_7_a();      // 0xFF

    void undefined();

    void inc(uint8_t& reg);
    void dec(uint8_t& reg);
    void add_hl(uint16_t& reg);
    void add_a(uint8_t value);
    void adc(uint8_t value);
    void sub(uint8_t value);
    void sbc(uint8_t value);
    void and_(uint8_t value);
    void xor_(uint8_t value);
    void or_(uint8_t value);
    void cp(uint8_t value);
    void rlc(uint8_t& reg);
    void rrc(uint8_t& reg);
    void rl(uint8_t& reg);
    void rr(uint8_t& reg);
    void sla(uint8_t& reg);
    void sra(uint8_t& reg);
    void swap(uint8_t& reg);
    void srl(uint8_t& reg);
    void bit(uint8_t bit, uint8_t value);

    uint16_t pop();
    void push(uint16_t value);

    void setHalfCarryFlag(uint8_t value1, uint8_t value2, bool isAdd);
    void setCarryFlag(uint8_t value1, uint8_t value2, bool isAdd);
    void setCarryFlag(uint16_t value1, uint16_t value2, bool isAdd);
    void setZeroFlag(uint8_t value);
};

}  // namespace qnemu
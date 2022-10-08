/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdio>

#include <cstdint>
#include <cstring>
#include <thread>

#include "qnemu/gb/cpu/GbCpu.h"

namespace qnemu
{

GbCpu::GbCpu() :
    instructions({
        Instruction
        { "NOP",                     1, 4,  [this](){nop();}       },
        { "LD BC, 0x%04X",           3, 12, [this](){ld_bc_nn();}  },
        { "LD (BC), A",              1, 8,  [this](){ld_bcp_a();}  },
        { "INC BC",                  1, 8,  [this](){inc_bc();}    },                     // 0x03
        { "INC B",                   1, 4,  [this](){inc_b();}     },                       // 0x04
        { "DEC B",                   1, 4,  [this](){dec_b();}     },                       // 0x05
        { "LD B, 0x%02X",            2, 8,  [this](){ld_b_n();}    },               // 0x06
        { "RLCA",                    1, 4,  [this](){rlca();}      },                         // 0x07
        { "LD (0x%04X), SP",         3, 20, [this](){ld_nnp_sp();} },         // 0x08
        { "ADD HL, BC",              1, 8,  [this](){add_hl_bc();} },              // 0x09
        { "LD A, (BC)",              1, 8,  [this](){ld_a_bcp();}  },               // 0x0a
        { "DEC BC",                  1, 8,  [this](){dec_bc();}    },                     // 0x0b
        { "INC C",                   1, 4,  [this](){inc_c();}     },                       // 0x0c
        { "DEC C",                   1, 4,  [this](){dec_c();}     },                       // 0x0d
        { "LD C, 0x%02X",            2, 8,  [this](){ld_c_n();}    },               // 0x0e
        { "RRCA",                    1, 4,  [this](){rrca();}      },                         // 0x0f
        { "STOP",                    2, 4,  [this](){stop_p();}    },                         // 0x10
        { "LD DE, 0x%04X",           3, 12, [this](){ld_de_nn();}  },            // 0x11
        { "LD (DE), A",              1, 8,  [this](){ld_dep_a();}  },               // 0x12
        { "INC DE",                  1, 8,  [this](){inc_de();}    },                     // 0x13
        { "INC D",                   1, 4,  [this](){inc_d();}     },                       // 0x14
        { "DEC D",                   1, 4,  [this](){dec_d();}     },                       // 0x15
        { "LD D, 0x%02X",            2, 8,  [this](){ld_d_n();}    },               // 0x16
        { "RLA",                     1, 4,  [this](){rla();}       },                           // 0x17
        { "JR 0x%02X",               2, 12, [this](){jr_n();}      },                    // 0x18
        { "ADD HL, DE",              1, 8,  [this](){add_hl_de();} },              // 0x19
        { "LD A, (DE)",              1, 8,  [this](){ld_a_dep();}  },               // 0x1a
        { "DEC DE",                  1, 8,  [this](){dec_de();}    },                     // 0x1b
        { "INC E",                   1, 4,  [this](){inc_e();}     },                       // 0x1c
        { "DEC E",                   1, 4,  [this](){dec_e();}     },                       // 0x1d
        { "LD E, 0x%02X",            2, 8,  [this](){ld_e_n();}    },               // 0x1e
        { "RRA",                     1, 4,  [this](){rra();}       },                           // 0x1f
        { "JR NZ, 0x%02X",           2, 8,  [this](){jr_nz_n();}   },             // 0x20
        { "LD HL, 0x%04X",           3, 12, [this](){ld_hl_nn();}  },            // 0x21
        { "LDI (HL), A",             1, 8,  [this](){ldi_hlp_a();} },             // 0x22
        { "INC HL",                  1, 8,  [this](){inc_hl();}    },                     // 0x23
        { "INC H",                   1, 4,  [this](){inc_h();}     },                       // 0x24
        { "DEC H",                   1, 4,  [this](){dec_h();}     },                       // 0x25
        { "LD H, 0x%02X",            2, 8,  [this](){ld_h_n();}    },               // 0x26
        { "DAA",                     1, 4,  [this](){daa();}       },                           // 0x27
        { "JR Z, 0x%02X",            2, 8,  [this](){jr_z_n();}    },               // 0x28
        { "ADD HL, HL",              1, 8,  [this](){add_hl_hl();} },              // 0x29
        { "LDI A, (HL)",             1, 8,  [this](){ldi_a_hlp();} },             // 0x2a
        { "DEC HL",                  1, 8,  [this](){dec_hl();}    },                     // 0x2b
        { "INC L",                   1, 4,  [this](){inc_l();}     },                       // 0x2c
        { "DEC L",                   1, 4,  [this](){dec_l();}     },                       // 0x2d
        { "LD L, 0x%02X",            2, 8,  [this](){ld_l_n();}    },               // 0x2e
        { "CPL",                     1, 4,  [this](){cpl();}       },                           // 0x2f
        { "JR NC, 0x%02X",           2, 8,  [this](){jr_nc_n();}   },             // 0x30
        { "LD SP, 0x%04X",           3, 12, [this](){ld_sp_nn();}  },            // 0x31
        { "LDD (HL), A",             1, 8,  [this](){ldd_hlp_a();} },             // 0x32
        { "INC SP",                  1, 8,  [this](){inc_sp();}    },                     // 0x33
        { "INC (HL)",                1, 12, [this](){inc_hlp();}   },                  // 0x34
        { "DEC (HL)",                1, 12, [this](){dec_hlp();}   },                  // 0x35
        { "LD (HL), 0x%02X",         2, 12, [this](){ld_hlp_n();}  },          // 0x36
        { "SCF",                     1, 4,  [this](){scf();}       },                           // 0x37
        { "JR C, 0x%02X",            2, 8,  [this](){jr_c_n();}    },               // 0x38
        { "ADD HL, SP",              1, 8,  [this](){add_hl_sp();} },              // 0x39
        { "LDD A, (HL)",             1, 8,  [this](){ldd_a_hlp();} },             // 0x3a
        { "DEC SP",                  1, 8,  [this](){dec_sp();}    },                     // 0x3b
        { "INC A",                   1, 4,  [this](){inc_a();}     },                       // 0x3c
        { "DEC A",                   1, 4,  [this](){dec_a();}     },                       // 0x3d
        { "LD A, 0x%02X",            2, 8,  [this](){ld_a_n();}    },               // 0x3e
        { "CCF",                     1, 4,  [this](){ccf();}       },                           // 0x3f
        { "LD B, B",                 1, 4,  [this](){ld_b_b();}    },                       // 0x40
        { "LD B, C",                 1, 4,  [this](){ld_b_c();}    },                    // 0x41
        { "LD B, D",                 1, 4,  [this](){ld_b_d();}    },                    // 0x42
        { "LD B, E",                 1, 4,  [this](){ld_b_e();}    },                    // 0x43
        { "LD B, H",                 1, 4,  [this](){ld_b_h();}    },                    // 0x44
        { "LD B, L",                 1, 4,  [this](){ld_b_l();}    },                    // 0x45
        { "LD B, (HL)",              1, 8,  [this](){ld_b_hlp();}  },               // 0x46
        { "LD B, A",                 1, 4,  [this](){ld_b_a();}    },                    // 0x47
        { "LD C, B",                 1, 4,  [this](){ld_c_b();}    },                    // 0x48
        { "LD C, C",                 1, 4,  [this](){ld_c_c();}    },                       // 0x49
        { "LD C, D",                 1, 4,  [this](){ld_c_d();}    },                    // 0x4a
        { "LD C, E",                 1, 4,  [this](){ld_c_e();}    },                    // 0x4b
        { "LD C, H",                 1, 4,  [this](){ld_c_h();}    },                    // 0x4c
        { "LD C, L",                 1, 4,  [this](){ld_c_l();}    },                    // 0x4d
        { "LD C, (HL)",              1, 8,  [this](){ld_c_hlp();}  },               // 0x4e
        { "LD C, A",                 1, 4,  [this](){ld_c_a();}    },                    // 0x4f
        { "LD D, B",                 1, 4,  [this](){ld_d_b();}    },                    // 0x50
        { "LD D, C",                 1, 4,  [this](){ld_d_c();}    },                    // 0x51
        { "LD D, D",                 1, 4,  [this](){ld_d_d();}    },                       // 0x52
        { "LD D, E",                 1, 4,  [this](){ld_d_e();}    },                    // 0x53
        { "LD D, H",                 1, 4,  [this](){ld_d_h();}    },                    // 0x54
        { "LD D, L",                 1, 4,  [this](){ld_d_l();}    },                    // 0x55
        { "LD D, (HL)",              1, 8,  [this](){ld_d_hlp();}  },               // 0x56
        { "LD D, A",                 1, 4,  [this](){ld_d_a();}    },                    // 0x57
        { "LD E, B",                 1, 4,  [this](){ld_e_b();}    },                    // 0x58
        { "LD E, C",                 1, 4,  [this](){ld_e_c();}    },                    // 0x59
        { "LD E, D",                 1, 4,  [this](){ld_e_d();}    },                    // 0x5a
        { "LD E, E",                 1, 4,  [this](){ld_e_e();}    },                       // 0x5b
        { "LD E, H",                 1, 4,  [this](){ld_e_h();}    },                    // 0x5c
        { "LD E, L",                 1, 4,  [this](){ld_e_l();}    },                    // 0x5d
        { "LD E, (HL)",              1, 8,  [this](){ld_e_hlp();}  },               // 0x5e
        { "LD E, A",                 1, 4,  [this](){ld_e_a();}    },                    // 0x5f
        { "LD H, B",                 1, 4,  [this](){ld_h_b();}    },                    // 0x60
        { "LD H, C",                 1, 4,  [this](){ld_h_c();}    },                    // 0x61
        { "LD H, D",                 1, 4,  [this](){ld_h_d();}    },                    // 0x62
        { "LD H, E",                 1, 4,  [this](){ld_h_e();}    },                    // 0x63
        { "LD H, H",                 1, 4,  [this](){ld_h_h();}    },                       // 0x64
        { "LD H, L",                 1, 4,  [this](){ld_h_l();}    },                    // 0x65
        { "LD H, (HL)",              1, 8,  [this](){ld_h_hlp();}  },               // 0x66
        { "LD H, A",                 1, 4,  [this](){ld_h_a();}    },                    // 0x67
        { "LD L, B",                 1, 4,  [this](){ld_l_b();}    },                    // 0x68
        { "LD L, C",                 1, 4,  [this](){ld_l_c();}    },                    // 0x69
        { "LD L, D",                 1, 4,  [this](){ld_l_d();}    },                    // 0x6a
        { "LD L, E",                 1, 4,  [this](){ld_l_e();}    },                    // 0x6b
        { "LD L, H",                 1, 4,  [this](){ld_l_h();}    },                    // 0x6c
        { "LD L, L",                 1, 4,  [this](){ld_l_l();}    },                       // 0x6d
        { "LD L, (HL)",              1, 8,  [this](){ld_l_hlp();}  },               // 0x6e
        { "LD L, A",                 1, 4,  [this](){ld_l_a();}    },                    // 0x6f
        { "LD (HL), B",              1, 8,  [this](){ld_hlp_b();}  },               // 0x70
        { "LD (HL), C",              1, 8,  [this](){ld_hlp_c();}  },               // 0x71
        { "LD (HL), D",              1, 8,  [this](){ld_hlp_d();}  },               // 0x72
        { "LD (HL), E",              1, 8,  [this](){ld_hlp_e();}  },               // 0x73
        { "LD (HL), H",              1, 8,  [this](){ld_hlp_h();}  },               // 0x74
        { "LD (HL), L",              1, 8,  [this](){ld_hlp_l();}  },               // 0x75
        { "HALT",                    1, 4,  [this](){halt();}      },                         // 0x76
        { "LD (HL), A",              1, 8,  [this](){ld_hlp_a();}  },               // 0x77
        { "LD A, B",                 1, 4,  [this](){ld_a_b();}    },                    // 0x78
        { "LD A, C",                 1, 4,  [this](){ld_a_c();}    },                    // 0x79
        { "LD A, D",                 1, 4,  [this](){ld_a_d();}    },                    // 0x7a
        { "LD A, E",                 1, 4,  [this](){ld_a_e();}    },                    // 0x7b
        { "LD A, H",                 1, 4,  [this](){ld_a_h();}    },                    // 0x7c
        { "LD A, L",                 1, 4,  [this](){ld_a_l();}    },                    // 0x7d
        { "LD A, (HL)",              1, 8,  [this](){ld_a_hlp();}  },               // 0x7e
        { "LD A, A",                 1, 4,  [this](){ld_a_a();}    },                       // 0x7f
        { "ADD A, B",                1, 4,  [this](){add_a_b();}   },                  // 0x80
        { "ADD A, C",                1, 4,  [this](){add_a_c();}   },                  // 0x81
        { "ADD A, D",                1, 4,  [this](){add_a_d();}   },                  // 0x82
        { "ADD A, E",                1, 4,  [this](){add_a_e();}   },                  // 0x83
        { "ADD A, H",                1, 4,  [this](){add_a_h();}   },                  // 0x84
        { "ADD A, L",                1, 4,  [this](){add_a_l();}   },                  // 0x85
        { "ADD A, (HL)",             1, 8,  [this](){add_a_hlp();} },             // 0x86
        { "ADD A",                   1, 4,  [this](){add_a_a();}   },                     // 0x87
        { "ADC B",                   1, 4,  [this](){adc_b();}     },                       // 0x88
        { "ADC C",                   1, 4,  [this](){adc_c();}     },                       // 0x89
        { "ADC D",                   1, 4,  [this](){adc_d();}     },                       // 0x8a
        { "ADC E",                   1, 4,  [this](){adc_e();}     },                       // 0x8b
        { "ADC H",                   1, 4,  [this](){adc_h();}     },                       // 0x8c
        { "ADC L",                   1, 4,  [this](){adc_l();}     },                       // 0x8d
        { "ADC (HL)",                1, 4,  [this](){adc_hlp();}   },                  // 0x8e
        { "ADC A",                   1, 4,  [this](){adc_a();}     },                       // 0x8f
        { "SUB B",                   1, 4,  [this](){sub_b();}     },                       // 0x90
        { "SUB C",                   1, 4,  [this](){sub_c();}     },                       // 0x91
        { "SUB D",                   1, 4,  [this](){sub_d();}     },                       // 0x92
        { "SUB E",                   1, 4,  [this](){sub_e();}     },                       // 0x93
        { "SUB H",                   1, 4,  [this](){sub_h();}     },                       // 0x94
        { "SUB L",                   1, 4,  [this](){sub_l();}     },                       // 0x95
        { "SUB (HL)",                1, 8,  [this](){sub_hlp();}   },                  // 0x96
        { "SUB A",                   1, 4,  [this](){sub_a();}     },                       // 0x97
        { "SBC B",                   1, 4,  [this](){sbc_b();}     },                       // 0x98
        { "SBC C",                   1, 4,  [this](){sbc_c();}     },                       // 0x99
        { "SBC D",                   1, 4,  [this](){sbc_d();}     },                       // 0x9a
        { "SBC E",                   1, 4,  [this](){sbc_e();}     },                       // 0x9b
        { "SBC H",                   1, 4,  [this](){sbc_h();}     },                       // 0x9c
        { "SBC L",                   1, 4,  [this](){sbc_l();}     },                       // 0x9d
        { "SBC (HL)",                1, 8,  [this](){sbc_hlp();}   },                  // 0x9e
        { "SBC A",                   1, 4,  [this](){sbc_a();}     },                       // 0x9f
        { "AND B",                   1, 4,  [this](){and_b();}     },                       // 0xa0
        { "AND C",                   1, 4,  [this](){and_c();}     },                       // 0xa1
        { "AND D",                   1, 4,  [this](){and_d();}     },                       // 0xa2
        { "AND E",                   1, 4,  [this](){and_e();}     },                       // 0xa3
        { "AND H",                   1, 4,  [this](){and_h();}     },                       // 0xa4
        { "AND L",                   1, 4,  [this](){and_l();}     },                       // 0xa5
        { "AND (HL)",                1, 8,  [this](){and_hlp();}   },                  // 0xa6
        { "AND A",                   1, 4,  [this](){and_a();}     },                       // 0xa7
        { "XOR B",                   1, 4,  [this](){xor_b();}     },                       // 0xa8
        { "XOR C",                   1, 4,  [this](){xor_c();}     },                       // 0xa9
        { "XOR D",                   1, 4,  [this](){xor_d();}     },                       // 0xaa
        { "XOR E",                   1, 4,  [this](){xor_e();}     },                       // 0xab
        { "XOR H",                   1, 4,  [this](){xor_h();}     },                       // 0xac
        { "XOR L",                   1, 4,  [this](){xor_l();}     },                       // 0xad
        { "XOR (HL)",                1, 8,  [this](){xor_hlp();}   },                  // 0xae
        { "XOR A",                   1, 4,  [this](){xor_a();}     },                       // 0xaf
        { "OR B",                    1, 4,  [this](){or_b();}      },                         // 0xb0
        { "OR C",                    1, 4,  [this](){or_c();}      },                         // 0xb1
        { "OR D",                    1, 4,  [this](){or_d();}      },                         // 0xb2
        { "OR E",                    1, 4,  [this](){or_e();}      },                         // 0xb3
        { "OR H",                    1, 4,  [this](){or_h();}      },                         // 0xb4
        { "OR L",                    1, 4,  [this](){or_l();}      },                         // 0xb5
        { "OR (HL)",                 1, 8,  [this](){or_hlp();}    },                    // 0xb6
        { "OR A",                    1, 4,  [this](){or_a();}      },                         // 0xb7
        { "CP B",                    1, 4,  [this](){cp_b();}      },                         // 0xb8
        { "CP C",                    1, 4,  [this](){cp_c();}      },                         // 0xb9
        { "CP D",                    1, 4,  [this](){cp_d();}      },                         // 0xba
        { "CP E",                    1, 4,  [this](){cp_e();}      },                         // 0xbb
        { "CP H",                    1, 4,  [this](){cp_h();}      },                         // 0xbc
        { "CP L",                    1, 4,  [this](){cp_l();}      },                         // 0xbd
        { "CP (HL)",                 1, 8,  [this](){cp_hlp();}    },                    // 0xbe
        { "CP A",                    1, 4,  [this](){cp_a();}      },                         // 0xbf
        { "RET NZ",                  1, 8,  [this](){ret_nz();}    },                     // 0xc0
        { "POP BC",                  1, 12, [this](){pop_bc();}    },                     // 0xc1
        { "JP NZ, 0x%04X",           3, 12, [this](){jp_nz_nn();}  },            // 0xc2
        { "JP 0x%04X",               3, 16, [this](){jp_nn();}     },                   // 0xc3
        { "CALL NZ, 0x%04X",         3, 12, [this](){call_nz_nn();}},        // 0xc4
        { "PUSH BC",                 1, 16, [this](){push_bc();}   },                   // 0xc5
        { "ADD A, 0x%02X",           2, 8,  [this](){add_a_n();}   },             // 0xc6
        { "RST 0x00",                1, 16, [this](){rst_0();}     },                    // 0xc7
        { "RET Z",                   1, 8,  [this](){ret_z();}     },                       // 0xc8
        { "RET",                     1, 16, [this](){ret();}       },                           // 0xc9
        { "JP Z, 0x%04X",            3, 12, [this](){jp_z_nn();}   },              // 0xca
        { "CB %02X",                 0, 0,  [this](){cb_n();}      },                      // 0xcb
        { "CALL Z, 0x%04X",          3, 12, [this](){call_z_nn();} },          // 0xcc
        { "CALL 0x%04X",             3, 24, [this](){call_nn();}   },               // 0xcd
        { "ADC 0x%02X",              2, 8,  [this](){adc_n();}     },                  // 0xce
        { "RST 0x08",                1, 16, [this](){rst_08();}    },                   // 0xcf
        { "RET NC",                  1, 8,  [this](){ret_nc();}    },                     // 0xd0
        { "POP DE",                  1, 12, [this](){pop_de();}    },                     // 0xd1
        { "JP NC, 0x%04X",           3, 12, [this](){jp_nc_nn();}  },            // 0xd2
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xd3
        { "CALL NC, 0x%04X",         3, 12, [this](){call_nc_nn();}},        // 0xd4
        { "PUSH DE",                 1, 16, [this](){push_de();}   },                   // 0xd5
        { "SUB 0x%02X",              2, 8,  [this](){sub_n();}     },                  // 0xd6
        { "RST 0x10",                1, 16, [this](){rst_10();}    },                   // 0xd7
        { "RET C",                   1, 8,  [this](){ret_c();}     },                       // 0xd8
        { "RETI",                    1, 16, [this](){reti();}      },          // 0xd9
        { "JP C, 0x%04X",            3, 12, [this](){jp_c_nn();}   },              // 0xda
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xdb
        { "CALL C, 0x%04X",          3, 12, [this](){call_c_nn();} },          // 0xdc
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xdd
        { "SBC 0x%02X",              2, 8,  [this](){sbc_n();}     },                  // 0xde
        { "RST 0x18",                1, 16, [this](){rst_18();}    },                   // 0xdf
        { "LD (0xFF00 + 0x%02X), A", 2, 12, [this](){ld_ff_n_ap();}},  // 0xe0
        { "POP HL",                  1, 12, [this](){pop_hl();}    },                     // 0xe1
        { "LD (0xFF00 + C), A",      1, 8,  [this](){ld_ff_c_a();} },      // 0xe2
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xe3
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xe4
        { "PUSH HL",                 1, 16, [this](){push_hl();}   },                   // 0xe5
        { "AND 0x%02X",              2, 8,  [this](){and_n();}     },                  // 0xe6
        { "RST 0x20",                1, 16, [this](){rst_20();}    },                   // 0xe7
        { "ADD SP,0x%02X",           2, 16, [this](){add_sp_n();}  },            // 0xe8
        { "JP HL",                   1, 4,  [this](){jp_hl();}     },                       // 0xe9
        { "LD (0x%04X), A",          3, 16, [this](){ld_nnp_a();}  },           // 0xea
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xeb
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xec
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xed
        { "XOR 0x%02X",              2, 8,  [this](){xor_n();}     },                  // 0xee
        { "RST 0x28",                1, 16, [this](){rst_28();}    },                   // 0xef
        { "LD A, (0xFF00 + 0x%02X)", 2, 12, [this](){ld_ff_ap_n();}},  // 0xf0
        { "POP AF",                  1, 12, [this](){pop_af();}    },                     // 0xf1
        { "LD A, (0xFF00 + C)",      1, 8,  [this](){ld_a_ff_c();} },      // 0xf2
        { "DI",                      1, 4,  [this](){di_inst();}   },                        // 0xf3
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xf4
        { "PUSH AF",                 1, 16, [this](){push_af();}   },                   // 0xf5
        { "OR 0x%02X",               2, 8,  [this](){or_n();}      },                    // 0xf6
        { "RST 0x30",                1, 16, [this](){rst_30();}    },                   // 0xf7
        { "LD HL, SP+0x%02X",        2, 12, [this](){ld_hl_sp_n();}},       // 0xf8
        { "LD SP, HL",               1, 8,  [this](){ld_sp_hl();}  },                // 0xf9
        { "LD A, (0x%04X)",          3, 16, [this](){ld_a_nnp();}  },           // 0xfa
        { "EI",                      1, 4,  [this](){ei();}        },                             // 0xfb
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xfc
        { "UNKNOWN",                 0, 0,  [this](){undefined();} },                 // 0xfd
        { "CP 0x%02X",               2, 8,  [this](){cp_n();}      },                    // 0xfe
        { "RST 0x38",                1, 16, [this](){rst_38();}    },                   // 0xff
    }),
    prefixedInstructions({
        Instruction
        { "RLC B",                   2, 8,  [this](){rlc_b();}     },
        { "RLC C",                   2, 8,  [this](){rlc_c();}     },
        { "RLC D",                   2, 8,  [this](){rlc_d();}     },
        { "RLC E",                   2, 8,  [this](){rlc_e();}     },                     // 0x03
        { "RLC H",                   2, 8,  [this](){rlc_h();}     },                       // 0x04
        { "RLC L",                   2, 8,  [this](){rlc_l();}     },                       // 0x05
        { "RLC (HL)",                2, 16, [this](){rlc_hlp();}   },               // 0x06
        { "RLC A",                   2, 8,  [this](){rlc_a();}     },                         // 0x07
        { "RRC B",                   2, 8,  [this](){rrc_b();}     },         // 0x08
        { "RRC C",                   2, 8,  [this](){rrc_c();}     },              // 0x09
        { "RRC D",                   2, 8,  [this](){rrc_d();}     },               // 0x0a
        { "RRC E",                   2, 8,  [this](){rrc_e();}     },                     // 0x0b
        { "RRC H",                   2, 8,  [this](){rrc_h();}     },                       // 0x0c
        { "RRC L",                   2, 8,  [this](){rrc_l();}     },                       // 0x0d
        { "RRC (HL)",                2, 16, [this](){rrc_hlp();}   },               // 0x0e
        { "RRC A",                   2, 8,  [this](){rrc_a();}     },                         // 0x0f
        { "RL B",                    2, 8,  [this](){rl_b();}      },             // 0x10
        { "RL C",                    2, 8,  [this](){rl_c();}      },             // 0x11
        { "RL D",                    2, 8,  [this](){rl_d();}      },             // 0x12
        { "RL E",                    2, 8,  [this](){rl_e();}      },             // 0x13
        { "RL H",                    2, 8,  [this](){rl_h();}      },             // 0x14
        { "RL L",                    2, 8,  [this](){rl_l();}      },             // 0x15
        { "RL (HL)",                 2, 16, [this](){rl_hlp();}    },        // 0x16
        { "RL A",                    2, 8,  [this](){rl_a();}      },             // 0x17
        { "RR B",                    2, 8,  [this](){rr_b();}      },             // 0x18
        { "RR C",                    2, 8,  [this](){rr_c();}      },             // 0x19
        { "RR D",                    2, 8,  [this](){rr_d();}      },             // 0x1a
        { "RR E",                    2, 8,  [this](){rr_e();}      },             // 0x1b
        { "RR H",                    2, 8,  [this](){rr_h();}      },             // 0x1c
        { "RR L",                    2, 8,  [this](){rr_l();}      },             // 0x1d
        { "RR (HL)",                 2, 16, [this](){rr_hlp();}    },        // 0x1e
        { "RR A",                    2, 8,  [this](){rr_a();}      },             // 0x1f
        { "SLA B",                   2, 8,  [this](){sla_b();}     },           // 0x20
        { "SLA C",                   2, 8,  [this](){sla_c();}     },           // 0x21
        { "SLA D",                   2, 8,  [this](){sla_d();}     },           // 0x22
        { "SLA E",                   2, 8,  [this](){sla_e();}     },           // 0x23
        { "SLA H",                   2, 8,  [this](){sla_h();}     },           // 0x24
        { "SLA L",                   2, 8,  [this](){sla_l();}     },           // 0x25
        { "SLA (HL)",                2, 16, [this](){sla_hlp();}   },      // 0x26
        { "SLA A",                   2, 8,  [this](){sla_a();}     },           // 0x27
        { "SRA B",                   2, 8,  [this](){sra_b();}     },           // 0x28
        { "SRA C",                   2, 8,  [this](){sra_c();}     },           // 0x29
        { "SRA D",                   2, 8,  [this](){sra_d();}     },           // 0x2a
        { "SRA E",                   2, 8,  [this](){sra_e();}     },           // 0x2b
        { "SRA H",                   2, 8,  [this](){sra_h();}     },           // 0x2c
        { "SRA L",                   2, 8,  [this](){sra_l();}     },           // 0x2d
        { "SRA (HL)",                2, 16, [this](){sra_hlp();}   },      // 0x2e
        { "SRA A",                   2, 8,  [this](){sra_a();}     },           // 0x2f
        { "SWAP B",                  2, 8,  [this](){swap_b();}    },         // 0x30
        { "SWAP C",                  2, 8,  [this](){swap_c();}    },         // 0x31
        { "SWAP D",                  2, 8,  [this](){swap_d();}    },         // 0x32
        { "SWAP E",                  2, 8,  [this](){swap_e();}    },         // 0x33
        { "SWAP H",                  2, 8,  [this](){swap_h();}    },         // 0x34
        { "SWAP L",                  2, 8,  [this](){swap_l();}    },         // 0x35
        { "SWAP (HL)",               2, 16, [this](){swap_hlp();}  },    // 0x36
        { "SWAP A",                  2, 8,  [this](){swap_a();}    },         // 0x37
        { "SRL B",                   2, 8,  [this](){srl_b();}     },           // 0x38
        { "SRL C",                   2, 8,  [this](){srl_c();}     },           // 0x39
        { "SRL D",                   2, 8,  [this](){srl_d();}     },           // 0x3a
        { "SRL E",                   2, 8,  [this](){srl_e();}     },           // 0x3b
        { "SRL H",                   2, 8,  [this](){srl_h();}     },           // 0x3c
        { "SRL L",                   2, 8,  [this](){srl_l();}     },           // 0x3d
        { "SRL (HL)",                2, 16, [this](){srl_hlp();}   },      // 0x3e
        { "SRL A",                   2, 8,  [this](){srl_a();}     },           // 0x3f
        { "BIT 0, B",                2, 8,  [this](){bit_0_b();}   },      // 0x40
        { "BIT 0, C",                2, 8,  [this](){bit_0_c();}   },      // 0x41
        { "BIT 0, D",                2, 8,  [this](){bit_0_d();}   },      // 0x42
        { "BIT 0, E",                2, 8,  [this](){bit_0_e();}   },      // 0x43
        { "BIT 0, H",                2, 8,  [this](){bit_0_h();}   },      // 0x44
        { "BIT 0, L",                2, 8,  [this](){bit_0_l();}   },      // 0x45
        { "BIT 0, (HL)",             2, 12, [this](){bit_0_hlp();} },  // 0x46
        { "BIT 0, A",                2, 8,  [this](){bit_0_a();}   },      // 0x47
        { "BIT 1, B",                2, 8,  [this](){bit_1_b();}   },      // 0x48
        { "BIT 1, C",                2, 8,  [this](){bit_1_c();}   },      // 0x49
        { "BIT 1, D",                2, 8,  [this](){bit_1_d();}   },      // 0x4a
        { "BIT 1, E",                2, 8,  [this](){bit_1_e();}   },      // 0x4b
        { "BIT 1, H",                2, 8,  [this](){bit_1_h();}   },      // 0x4c
        { "BIT 1, L",                2, 8,  [this](){bit_1_l();}   },      // 0x4d
        { "BIT 1, (HL)",             2, 12, [this](){bit_1_hlp();} },  // 0x4e
        { "BIT 1, A",                2, 8,  [this](){bit_1_a();}   },      // 0x4f
        { "BIT 2, B",                2, 8,  [this](){bit_2_b();}   },      // 0x50
        { "BIT 2, C",                2, 8,  [this](){bit_2_c();}   },      // 0x51
        { "BIT 2, D",                2, 8,  [this](){bit_2_d();}   },      // 0x52
        { "BIT 2, E",                2, 8,  [this](){bit_2_e();}   },      // 0x53
        { "BIT 2, H",                2, 8,  [this](){bit_2_h();}   },      // 0x54
        { "BIT 2, L",                2, 8,  [this](){bit_2_l();}   },      // 0x55
        { "BIT 2, (HL)",             2, 12, [this](){bit_2_hlp();} },  // 0x56
        { "BIT 2, A",                2, 8,  [this](){bit_2_a();}   },      // 0x57
        { "BIT 3, B",                2, 8,  [this](){bit_3_b();}   },      // 0x58
        { "BIT 3, C",                2, 8,  [this](){bit_3_c();}   },      // 0x59
        { "BIT 3, D",                2, 8,  [this](){bit_3_d();}   },      // 0x5a
        { "BIT 3, E",                2, 8,  [this](){bit_3_e();}   },      // 0x5b
        { "BIT 3, H",                2, 8,  [this](){bit_3_h();}   },      // 0x5c
        { "BIT 3, L",                2, 8,  [this](){bit_3_l();}   },      // 0x5d
        { "BIT 3, (HL)",             2, 12, [this](){bit_3_hlp();} },  // 0x5e
        { "BIT 3, A",                2, 8,  [this](){bit_3_a();}   },      // 0x5f
        { "BIT 4, B",                2, 8,  [this](){bit_4_b();}   },      // 0x60
        { "BIT 4, C",                2, 8,  [this](){bit_4_c();}   },      // 0x61
        { "BIT 4, D",                2, 8,  [this](){bit_4_d();}   },      // 0x62
        { "BIT 4, E",                2, 8,  [this](){bit_4_e();}   },      // 0x63
        { "BIT 4, H",                2, 8,  [this](){bit_4_h();}   },      // 0x64
        { "BIT 4, L",                2, 8,  [this](){bit_4_l();}   },      // 0x65
        { "BIT 4, (HL)",             2, 12, [this](){bit_4_hlp();} },  // 0x66
        { "BIT 4, A",                2, 8,  [this](){bit_4_a();}   },      // 0x67
        { "BIT 5, B",                2, 8,  [this](){bit_5_b();}   },      // 0x68
        { "BIT 5, C",                2, 8,  [this](){bit_5_c();}   },      // 0x69
        { "BIT 5, D",                2, 8,  [this](){bit_5_d();}   },      // 0x6a
        { "BIT 5, E",                2, 8,  [this](){bit_5_e();}   },      // 0x6b
        { "BIT 6, H",                2, 8,  [this](){bit_5_h();}   },      // 0x6c
        { "BIT 6, L",                2, 8,  [this](){bit_5_l();}   },      // 0x6d
        { "BIT 5, (HL)",             2, 12, [this](){bit_5_hlp();} },  // 0x6e
        { "BIT 5, A",                2, 8,  [this](){bit_5_a();}   },      // 0x6f
        { "BIT 6, B",                2, 8,  [this](){bit_6_b();}   },      // 0x70
        { "BIT 6, C",                2, 8,  [this](){bit_6_c();}   },      // 0x71
        { "BIT 6, D",                2, 8,  [this](){bit_6_d();}   },      // 0x72
        { "BIT 6, E",                2, 8,  [this](){bit_6_e();}   },      // 0x73
        { "BIT 6, H",                2, 8,  [this](){bit_6_h();}   },      // 0x74
        { "BIT 6, L",                2, 8,  [this](){bit_6_l();}   },      // 0x75
        { "BIT 6, (HL)",             2, 12, [this](){bit_6_hlp();} },  // 0x76
        { "BIT 6, A",                2, 8,  [this](){bit_6_a();}   },      // 0x77
        { "BIT 7, B",                2, 8,  [this](){bit_7_b();}   },      // 0x78
        { "BIT 7, C",                2, 8,  [this](){bit_7_c();}   },      // 0x79
        { "BIT 7, D",                2, 8,  [this](){bit_7_d();}   },      // 0x7a
        { "BIT 7, E",                2, 8,  [this](){bit_7_e();}   },      // 0x7b
        { "BIT 7, H",                2, 8,  [this](){bit_7_h();}   },      // 0x7c
        { "BIT 7, L",                2, 8,  [this](){bit_7_l();}   },      // 0x7d
        { "BIT 7, (HL)",             2, 12, [this](){bit_7_hlp();} },  // 0x7e
        { "BIT 7, A",                2, 8,  [this](){bit_7_a();}   },      // 0x7f
        { "RES 0, B",                2, 8,  [this](){res_0_b();}   },      // 0x80
        { "RES 0, C",                2, 8,  [this](){res_0_c();}   },      // 0x81
        { "RES 0, D",                2, 8,  [this](){res_0_d();}   },      // 0x82
        { "RES 0, E",                2, 8,  [this](){res_0_e();}   },      // 0x83
        { "RES 0, H",                2, 8,  [this](){res_0_h();}   },      // 0x84
        { "RES 0, L",                2, 8,  [this](){res_0_l();}   },      // 0x85
        { "RES 0, (HL)",             2, 16, [this](){res_0_hlp();} },  // 0x86
        { "RES 0, A",                2, 8,  [this](){res_0_a();}   },      // 0x87
        { "RES 1, B",                2, 8,  [this](){res_1_b();}   },      // 0x88
        { "RES 1, C",                2, 8,  [this](){res_1_c();}   },      // 0x89
        { "RES 1, D",                2, 8,  [this](){res_1_d();}   },      // 0x8a
        { "RES 1, E",                2, 8,  [this](){res_1_e();}   },      // 0x8b
        { "RES 1, H",                2, 8,  [this](){res_1_h();}   },      // 0x8c
        { "RES 1, L",                2, 8,  [this](){res_1_l();}   },      // 0x8d
        { "RES 1, (HL)",             2, 16, [this](){res_1_hlp();} },  // 0x8e
        { "RES 1, A",                2, 8,  [this](){res_1_a();}   },      // 0x8f
        { "RES 2, B",                2, 8,  [this](){res_2_b();}   },      // 0x90
        { "RES 2, C",                2, 8,  [this](){res_2_c();}   },      // 0x91
        { "RES 2, D",                2, 8,  [this](){res_2_d();}   },      // 0x92
        { "RES 2, E",                2, 8,  [this](){res_2_e();}   },      // 0x93
        { "RES 2, H",                2, 8,  [this](){res_2_h();}   },      // 0x94
        { "RES 2, L",                2, 8,  [this](){res_2_l();}   },      // 0x95
        { "RES 2, (HL)",             2, 16, [this](){res_2_hlp();} },  // 0x96
        { "RES 2, A",                2, 8,  [this](){res_2_a();}   },      // 0x97
        { "RES 3, B",                2, 8,  [this](){res_3_b();}   },      // 0x98
        { "RES 3, C",                2, 8,  [this](){res_3_c();}   },      // 0x99
        { "RES 3, D",                2, 8,  [this](){res_3_d();}   },      // 0x9a
        { "RES 3, E",                2, 8,  [this](){res_3_e();}   },      // 0x9b
        { "RES 3, H",                2, 8,  [this](){res_3_h();}   },      // 0x9c
        { "RES 3, L",                2, 8,  [this](){res_3_l();}   },      // 0x9d
        { "RES 3, (HL)",             2, 16, [this](){res_3_hlp();} },  // 0x9e
        { "RES 3, A",                2, 8,  [this](){res_3_a();}   },      // 0x9f
        { "RES 4, B",                2, 8,  [this](){res_4_b();}   },      // 0xa0
        { "RES 4, C",                2, 8,  [this](){res_4_c();}   },      // 0xa1
        { "RES 4, D",                2, 8,  [this](){res_4_d();}   },      // 0xa2
        { "RES 4, E",                2, 8,  [this](){res_4_e();}   },      // 0xa3
        { "RES 4, H",                2, 8,  [this](){res_4_h();}   },      // 0xa4
        { "RES 4, L",                2, 8,  [this](){res_4_l();}   },      // 0xa5
        { "RES 4, (HL)",             2, 16, [this](){res_4_hlp();} },  // 0xa6
        { "RES 4, A",                2, 8,  [this](){res_4_a();}   },      // 0xa7
        { "RES 5, B",                2, 8,  [this](){res_5_b();}   },      // 0xa8
        { "RES 5, C",                2, 8,  [this](){res_5_c();}   },      // 0xa9
        { "RES 5, D",                2, 8,  [this](){res_5_d();}   },      // 0xaa
        { "RES 5, E",                2, 8,  [this](){res_5_e();}   },      // 0xab
        { "RES 5, H",                2, 8,  [this](){res_5_h();}   },      // 0xac
        { "RES 5, L",                2, 8,  [this](){res_5_l();}   },      // 0xad
        { "RES 5, (HL)",             2, 16, [this](){res_5_hlp();} },  // 0xae
        { "RES 5, A",                2, 8,  [this](){res_5_a();}   },      // 0xaf
        { "RES 6, B",                2, 8,  [this](){res_6_b();}   },      // 0xb0
        { "RES 6, C",                2, 8,  [this](){res_6_c();}   },      // 0xb1
        { "RES 6, D",                2, 8,  [this](){res_6_d();}   },      // 0xb2
        { "RES 6, E",                2, 8,  [this](){res_6_e();}   },      // 0xb3
        { "RES 6, H",                2, 8,  [this](){res_6_h();}   },      // 0xb4
        { "RES 6, L",                2, 8,  [this](){res_6_l();}   },      // 0xb5
        { "RES 6, (HL)",             2, 16, [this](){res_6_hlp();} },  // 0xb6
        { "RES 6, A",                2, 8,  [this](){res_6_a();}   },      // 0xb7
        { "RES 7, B",                2, 8,  [this](){res_7_b();}   },      // 0xb8
        { "RES 7, C",                2, 8,  [this](){res_7_c();}   },      // 0xb9
        { "RES 7, D",                2, 8,  [this](){res_7_d();}   },      // 0xba
        { "RES 7, E",                2, 8,  [this](){res_7_e();}   },      // 0xbb
        { "RES 7, H",                2, 8,  [this](){res_7_h();}   },      // 0xbc
        { "RES 7, L",                2, 8,  [this](){res_7_l();}   },      // 0xbd
        { "RES 7, (HL)",             2, 16, [this](){res_7_hlp();} },  // 0xbe
        { "RES 7, A",                2, 8,  [this](){res_7_a();}   },      // 0xbf
        { "SET 0, B",                2, 8,  [this](){set_0_b();}   },      // 0xc0
        { "SET 0, C",                2, 8,  [this](){set_0_c();}   },      // 0xc1
        { "SET 0, D",                2, 8,  [this](){set_0_d();}   },      // 0xc2
        { "SET 0, E",                2, 8,  [this](){set_0_e();}   },      // 0xc3
        { "SET 0, H",                2, 8,  [this](){set_0_h();}   },      // 0xc4
        { "SET 0, L",                2, 8,  [this](){set_0_l();}   },      // 0xc5
        { "SET 0, (HL)",             2, 16, [this](){set_0_hlp();} },  // 0xc6
        { "SET 0, A",                2, 8,  [this](){set_0_a();}   },      // 0xc7
        { "SET 1, B",                2, 8,  [this](){set_1_b();}   },      // 0xc8
        { "SET 1, C",                2, 8,  [this](){set_1_c();}   },      // 0xc9
        { "SET 1, D",                2, 8,  [this](){set_1_d();}   },      // 0xca
        { "SET 1, E",                2, 8,  [this](){set_1_e();}   },      // 0xcb
        { "SET 1, H",                2, 8,  [this](){set_1_h();}   },      // 0xcc
        { "SET 1, L",                2, 8,  [this](){set_1_l();}   },      // 0xcd
        { "SET 1, (HL)",             2, 16, [this](){set_1_hlp();} },  // 0xce
        { "SET 1, A",                2, 8,  [this](){set_1_a();}   },      // 0xcf
        { "SET 2, B",                2, 8,  [this](){set_2_b();}   },      // 0xd0
        { "SET 2, C",                2, 8,  [this](){set_2_c();}   },      // 0xd1
        { "SET 2, D",                2, 8,  [this](){set_2_d();}   },      // 0xd2
        { "SET 2, E",                2, 8,  [this](){set_2_e();}   },      // 0xd3
        { "SET 2, H",                2, 8,  [this](){set_2_h();}   },      // 0xd4
        { "SET 2, L",                2, 8,  [this](){set_2_l();}   },      // 0xd5
        { "SET 2, (HL)",             2, 16, [this](){set_2_hlp();} },  // 0xd6
        { "SET 2, A",                2, 8,  [this](){set_2_a();}   },      // 0xd7
        { "SET 3, B",                2, 8,  [this](){set_3_b();}   },      // 0xd8
        { "SET 3, C",                2, 8,  [this](){set_3_c();}   },      // 0xd9
        { "SET 3, D",                2, 8,  [this](){set_3_d();}   },      // 0xda
        { "SET 3, E",                2, 8,  [this](){set_3_e();}   },      // 0xdb
        { "SET 3, H",                2, 8,  [this](){set_3_h();}   },      // 0xdc
        { "SET 3, L",                2, 8,  [this](){set_3_l();}   },      // 0xdd
        { "SET 3, (HL)",             2, 16, [this](){set_3_hlp();} },  // 0xde
        { "SET 3, A",                2, 8,  [this](){set_3_a();}   },      // 0xdf
        { "SET 4, B",                2, 8,  [this](){set_4_b();}   },      // 0xe0
        { "SET 4, C",                2, 8,  [this](){set_4_c();}   },      // 0xe1
        { "SET 4, D",                2, 8,  [this](){set_4_d();}   },      // 0xe2
        { "SET 4, E",                2, 8,  [this](){set_4_e();}   },      // 0xe3
        { "SET 4, H",                2, 8,  [this](){set_4_h();}   },      // 0xe4
        { "SET 4, L",                2, 8,  [this](){set_4_l();}   },      // 0xe5
        { "SET 4, (HL)",             2, 16, [this](){set_4_hlp();} },  // 0xe6
        { "SET 4, A",                2, 8,  [this](){set_4_a();}   },      // 0xe7
        { "SET 5, B",                2, 8,  [this](){set_5_b();}   },      // 0xe8
        { "SET 5, C",                2, 8,  [this](){set_5_c();}   },      // 0xe9
        { "SET 5, D",                2, 8,  [this](){set_5_d();}   },      // 0xea
        { "SET 5, E",                2, 8,  [this](){set_5_e();}   },      // 0xeb
        { "SET 5, H",                2, 8,  [this](){set_5_h();}   },      // 0xec
        { "SET 5, L",                2, 8,  [this](){set_5_l();}   },      // 0xed
        { "SET 5, (HL)",             2, 16, [this](){set_5_hlp();} },  // 0xee
        { "SET 5, A",                2, 8,  [this](){set_5_a();}   },      // 0xef
        { "SET 6, B",                2, 8,  [this](){set_6_b();}   },      // 0xf0
        { "SET 6, C",                2, 8,  [this](){set_6_c();}   },      // 0xf1
        { "SET 6, D",                2, 8,  [this](){set_6_d();}   },      // 0xf2
        { "SET 6, E",                2, 8,  [this](){set_6_e();}   },      // 0xf3
        { "SET 6, H",                2, 8,  [this](){set_6_h();}   },      // 0xf4
        { "SET 6, L",                2, 8,  [this](){set_6_l();}   },      // 0xf5
        { "SET 6, (HL)",             2, 16, [this](){set_6_hlp();} },  // 0xf6
        { "SET 6, A",                2, 8,  [this](){set_6_a();}   },      // 0xf7
        { "SET 7, B",                2, 8,  [this](){set_7_b();}   },      // 0xf8
        { "SET 7, C",                2, 8,  [this](){set_7_c();}   },      // 0xf9
        { "SET 7, D",                2, 8,  [this](){set_7_d();}   },      // 0xfa
        { "SET 7, E",                2, 8,  [this](){set_7_e();}   },      // 0xfb
        { "SET 7, H",                2, 8,  [this](){set_7_h();}   },      // 0xfc
        { "SET 7, L",                2, 8,  [this](){set_7_l();}   },      // 0xfd
        { "SET 7, (HL)",             2, 16, [this](){set_7_hlp();} },  // 0xfe
        { "SET 7, A",                2, 8,  [this](){set_7_a();}   },      // 0xff
    })
{
    GbCpu::reset();
}

GbCpu::~GbCpu()
{
    GbCpu::stop();
}

void GbCpu::start()
{
    if (!started.load()) {
        work = std::thread(&GbCpu::exec, this);
        started.store(true);
    }
}

void GbCpu::stop()
{
    started.store(false);
    if (work.joinable()) {
        work.join();
    }
}

void GbCpu::reset()
{
    memset(&registers, 0, sizeof(registers));
    registers.a = 0x11;
    registers.d = 0xFF;
    registers.e = 0x56;
    registers.l = 0x0D;
    registers.zero = 0x1;
    registers.fReserved = 0;
    registers.pc = 0x100;
    registers.sp = 0xFFFE;

    enableInterruptFlag = false;
    halt_mode = false;
    stop_mode = false;
    started = false;
    ticks = 0;
    for (auto& device : devices) {
        device->reset();
    }
}

bool GbCpu::isInHaltMode() const
{
    return halt_mode.load();
}

void GbCpu::exitHaltMode()
{
    halt_mode.store(false);
}

bool GbCpu::isInStopMode() const
{
    return stop_mode.load();
}

void GbCpu::exitStopMode()
{
    stop_mode.store(false);
    if (display) {
        display->enable();
    }
}

void GbCpu::jumpToAddress(uint16_t address)
{
    push(registers.pc);
    registers.pc = address;
    ticks = 20;
}

void GbCpu::cancelInterrupt()
{
    registers.pc = 0;
}

void GbCpu::addDisplay(std::shared_ptr<DisplayInterface> display)
{
    this->display = display;
}

void GbCpu::addDevice(std::shared_ptr<GbDeviceInterface> device)
{
    devices.push_back(device);
}

void GbCpu::exec()
{
    while(true) {
        if (!started.load()) {
            return;
        }
        if ((!halt_mode.load()) && (!stop_mode.load())) {
            step();
        }
        for (auto& device : devices) {
            device->step();
        }
    }
}

void GbCpu::step()
{
    if (!started.load()) {
        return;
    }
    if (ticks == 0) {
        if (enableInterruptFlag) {
            GbDeviceInterface::interruptMasterEnabled = true;
            enableInterruptFlag = false;
        }
        auto instruction = instructions.at(readByte(registers.pc));
        const uint16_t pc = registers.pc;
        ticks = instruction.ticks;
        instruction.execute();
        if (pc == registers.pc) {
            registers.pc += instruction.length;
        }
    } else {
        ticks--;
    }
}

uint8_t GbCpu::readByte(uint16_t address) const
{
    for (const auto& device : devices) {
        if (device->accepts(address)) {
            return device->read(address);
        }
    }
    return 0xFF;
}

void GbCpu::writeByte(uint16_t address, uint8_t value)
{
    for (auto& device : devices) {
        if (device->accepts(address)) {
            device->write(address, value);
        }
    }
}

uint16_t GbCpu::pop()
{
    const uint16_t value = readByte(registers.sp) | (static_cast<uint16_t>(readByte(registers.sp + 1)) << 8);
    registers.sp += 2;

    return value;
}

void GbCpu::push(uint16_t value)
{
    registers.sp -= 2;
    const uint8_t low = value & 0x00FF;
    const uint8_t high = ((value & 0xFF00) >> 8);
    writeByte(registers.sp, low);
    writeByte(registers.sp + 1, high);
}

void GbCpu::setHalfCarryFlag(uint8_t value1, uint8_t value2, bool isAdd) {
    if (isAdd) {
        const uint8_t result = (value1 & 0x0F) + (value2 & 0x0F);
        if (result & 0xF0) {
            registers.halfCarry = 1;
        }
        else {
            registers.halfCarry = 0;
        }
    }
    else {
        if ((value1 & 0x0F) < (value2 & 0x0F)) {
            registers.halfCarry = 1;
        }
        else {
            registers.halfCarry = 0;
        }
    }
}

void GbCpu::setCarryFlag(uint8_t value1, uint8_t value2, bool isAdd) {
    if (isAdd) {
        const uint16_t result = value1 + value2;
        if (result & 0xFF00) {
            registers.carry = 1;
        }
        else {
            registers.carry = 0;
        }
    }
    else {
        if (value1 < value2) {
            registers.carry = 1;
        }
        else {
            registers.carry = 0;
        }
    }
}

void GbCpu::setCarryFlag(uint16_t value1, uint16_t value2, bool isAdd) {
    if (isAdd) {
        const uint32_t result = value1 + value2;
        if (result & 0xFFFF0000) {
            registers.carry = 1;
        }
        else {
            registers.carry = 0;
        }
    }
    else {
        if (value1 < value2) {
            registers.carry = 1;
        }
        else {
            registers.carry = 0;
        }
    }
}

void GbCpu::setZeroFlag(uint8_t value) {
    if (value) {
        registers.zero = 0;
    }
    else {
        registers.zero = 1;
    }
}

}  // namespace qnemu
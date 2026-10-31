add_executable(
    gb_test_runner
    main.cpp
    GbTestSystem.cpp)
set_target_properties(gb_test_runner PROPERTIES
    AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(gb_test_runner PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_test_runner PRIVATE
    qnemu_warnings
    gb_bootrom
    gb_cpu
    gb_interrupt
    gb_ppu
    gb_apu
    gb_timer
    gb_cartridge
    Qt6::Gui)

set(QNEMU_TEST_ROM_ROOT "${PROJECT_SOURCE_DIR}/third_party" CACHE PATH
    "Directory containing the gb-test-roms and mooneye-test-roms submodules")
option(QNEMU_ENABLE_ROM_TESTS
    "Register the curated blargg/Mooneye test ROMs with CTest when available" ON)

function(qnemu_add_rom_test test_name rom_path label)
    if(NOT EXISTS "${rom_path}")
        return()
    endif()

    string(MAKE_C_IDENTIFIER "rom_${test_name}" identifier)
    add_test(
        NAME "${identifier}"
        COMMAND gb_test_runner "${rom_path}" --max-cycles 300000000 ${ARGN})
    set_tests_properties("${identifier}" PROPERTIES
        TIMEOUT 120
        LABELS "rom;${label}")
endfunction()

if(QNEMU_ENABLE_ROM_TESTS)
    set(BLARGG_ROM_ROOT "${QNEMU_TEST_ROM_ROOT}/gb-test-roms")
    if(EXISTS "${BLARGG_ROM_ROOT}/cpu_instrs")
        file(GLOB BLARGG_CPU_INDIVIDUAL "${BLARGG_ROM_ROOT}/cpu_instrs/individual/*.gb")
        foreach(ROM IN LISTS BLARGG_CPU_INDIVIDUAL)
            get_filename_component(NAME "${ROM}" NAME_WE)
            qnemu_add_rom_test("blargg_cpu_instrs_${NAME}" "${ROM}" "blargg")
        endforeach()
        qnemu_add_rom_test("blargg_cpu_instrs" "${BLARGG_ROM_ROOT}/cpu_instrs/cpu_instrs.gb" "blargg")

        qnemu_add_rom_test("blargg_instr_timing"
            "${BLARGG_ROM_ROOT}/instr_timing/instr_timing.gb" "blargg")

        qnemu_add_rom_test("blargg_halt_bug"
            "${BLARGG_ROM_ROOT}/halt_bug.gb" "blargg")

        qnemu_add_rom_test("blargg_interrupt_time"
            "${BLARGG_ROM_ROOT}/interrupt_time/interrupt_time.gb"
            "blargg" --model cgb --apu)

        file(GLOB BLARGG_MEM_TIMING "${BLARGG_ROM_ROOT}/mem_timing/individual/*.gb")
        foreach(ROM IN LISTS BLARGG_MEM_TIMING)
            get_filename_component(NAME "${ROM}" NAME_WE)
            qnemu_add_rom_test("blargg_mem_timing_${NAME}" "${ROM}" "blargg")
        endforeach()
        qnemu_add_rom_test("blargg_mem_timing" "${BLARGG_ROM_ROOT}/mem_timing/mem_timing.gb" "blargg")

        file(GLOB BLARGG_MEM_TIMING2 "${BLARGG_ROM_ROOT}/mem_timing-2/rom_singles/*.gb")
        foreach(ROM IN LISTS BLARGG_MEM_TIMING2)
            get_filename_component(NAME "${ROM}" NAME_WE)
            qnemu_add_rom_test("blargg_mem_timing2_${NAME}" "${ROM}" "blargg")
        endforeach()
        qnemu_add_rom_test("blargg_mem_timing2" "${BLARGG_ROM_ROOT}/mem_timing-2/mem_timing.gb" "blargg")

        # Sound register/length/trigger/sweep and wave-RAM timing tests.
        set(BLARGG_DMG_SOUND_TESTS
            "01-registers"
            "02-len ctr"
            "03-trigger"
            "04-sweep"
            "05-sweep details"
            "06-overflow on trigger"
            "07-len sweep period sync"
            "08-len ctr during power"
            "09-wave read while on"
            "10-wave trigger while on"
            "11-regs after power"
            "12-wave write while on"
        )
        foreach(NAME IN LISTS BLARGG_DMG_SOUND_TESTS)
            qnemu_add_rom_test("blargg_dmg_sound_${NAME}"
                "${BLARGG_ROM_ROOT}/dmg_sound/rom_singles/${NAME}.gb"
                "blargg;sound" --model dmg --apu)
        endforeach()

        set(BLARGG_CGB_SOUND_TESTS
            "01-registers"
            "02-len ctr"
            "03-trigger"
            "04-sweep"
            "05-sweep details"
            "06-overflow on trigger"
            "07-len sweep period sync"
            "08-len ctr during power"
            "09-wave read while on"
            "10-wave trigger while on"
            "11-regs after power"
            "12-wave"
        )
        foreach(NAME IN LISTS BLARGG_CGB_SOUND_TESTS)
            qnemu_add_rom_test("blargg_cgb_sound_${NAME}"
                "${BLARGG_ROM_ROOT}/cgb_sound/rom_singles/${NAME}.gb"
                "blargg;sound" --model cgb --apu)
        endforeach()
        # OAM corruption: causes, scanline timing and the effect-pattern tests.
        set(BLARGG_OAM_BUG_TESTS
            "1-lcd_sync"
            "2-causes"
            "3-non_causes"
            "4-scanline_timing"
            "5-timing_bug"
            "6-timing_no_bug"
            "8-instr_effect"
        )
        foreach(NAME IN LISTS BLARGG_OAM_BUG_TESTS)
            qnemu_add_rom_test("blargg_oam_bug_${NAME}"
                "${BLARGG_ROM_ROOT}/oam_bug/rom_singles/${NAME}.gb"
                "blargg;ppu" --model dmg --ppu)
        endforeach()

        # 7-timing_effect writes its optional text log past the 8 KiB
        # cartridge RAM (about 9.5 KiB of OAM dumps) and overwrites the code
        # copied to $C000.  The log is redundant with the console output and
        # is not part of the checksum, so disable write_text_out for this ROM.
        qnemu_add_rom_test("blargg_oam_bug_7-timing_effect"
            "${BLARGG_ROM_ROOT}/oam_bug/rom_singles/7-timing_effect.gb"
            "blargg;ppu" --model dmg --ppu --patch 0x4401=000000)
    endif()

    set(MOONEYE_ROM_ROOT "${QNEMU_TEST_ROM_ROOT}/mooneye-test-roms")
    if(EXISTS "${MOONEYE_ROM_ROOT}/acceptance")
        # Curated subset that the CPU/timer/interrupt test bus is expected to
        # pass.  Tests needing the real PPU, APU, boot state or an MBC have
        # their own lists below.
        set(MOONEYE_CPU_TESTS
            "acceptance/bits/reg_f.gb"
            "acceptance/div_timing.gb"
            "acceptance/ei_sequence.gb"
            "acceptance/ei_timing.gb"
            "acceptance/halt_ime0_ei.gb"
            "acceptance/halt_ime0_nointr_timing.gb"
            "acceptance/halt_ime1_timing.gb"
            "acceptance/halt_ime1_timing2-GS.gb"
            "acceptance/if_ie_registers.gb"
            "acceptance/instr/daa.gb"
            "acceptance/interrupts/ie_push.gb"
            "acceptance/intr_timing.gb"
            "acceptance/pop_timing.gb"
            "acceptance/rapid_di_ei.gb"
            "acceptance/reti_intr_timing.gb"
            "acceptance/timer/tim00.gb"
            "acceptance/timer/tim00_div_trigger.gb"
            "acceptance/timer/tim01.gb"
            "acceptance/timer/tim01_div_trigger.gb"
            "acceptance/timer/tim10.gb"
            "acceptance/timer/tim10_div_trigger.gb"
            "acceptance/timer/tim11.gb"
            "acceptance/timer/tim11_div_trigger.gb"
            "acceptance/timer/div_write.gb"
            "acceptance/timer/rapid_toggle.gb"
            "acceptance/timer/tima_reload.gb"
            "acceptance/timer/tima_write_reloading.gb"
            "acceptance/timer/tma_write_reloading.gb"
        )
        foreach(RELATIVE IN LISTS MOONEYE_CPU_TESTS)
            qnemu_add_rom_test("mooneye_${RELATIVE}" "${MOONEYE_ROM_ROOT}/${RELATIVE}" "mooneye")
        endforeach()

        # These need the real PPU (OAM visibility and OAM DMA), so attach it
        # instead of using the CPU-only LY/VBlank stub.
        set(MOONEYE_PPU_TESTS
            "acceptance/add_sp_e_timing.gb"
            "acceptance/call_cc_timing.gb"
            "acceptance/call_cc_timing2.gb"
            "acceptance/call_timing.gb"
            "acceptance/call_timing2.gb"
            "acceptance/di_timing-GS.gb"
            "acceptance/jp_cc_timing.gb"
            "acceptance/jp_timing.gb"
            "acceptance/ld_hl_sp_e_timing.gb"
            "acceptance/push_timing.gb"
            "acceptance/ret_cc_timing.gb"
            "acceptance/ret_timing.gb"
            "acceptance/reti_timing.gb"
            "acceptance/rst_timing.gb"
            "acceptance/bits/mem_oam.gb"
            "acceptance/oam_dma/basic.gb"
            "acceptance/oam_dma/reg_read.gb"
            "acceptance/oam_dma_start.gb"
            "acceptance/oam_dma_restart.gb"
            "acceptance/oam_dma_timing.gb"
            "acceptance/ppu/hblank_ly_scx_timing-GS.gb"
            "acceptance/ppu/intr_1_2_timing-GS.gb"
            "acceptance/ppu/intr_2_0_timing.gb"
            "acceptance/ppu/intr_2_mode0_timing.gb"
            "acceptance/ppu/intr_2_mode0_timing_sprites.gb"
            "acceptance/ppu/intr_2_mode3_timing.gb"
            "acceptance/ppu/intr_2_oam_ok_timing.gb"
            "acceptance/ppu/stat_irq_blocking.gb"
            "acceptance/ppu/stat_lyc_onoff.gb"
            "acceptance/ppu/vblank_stat_intr-GS.gb"
        )
        foreach(RELATIVE IN LISTS MOONEYE_PPU_TESTS)
            qnemu_add_rom_test("mooneye_ppu_${RELATIVE}"
                "${MOONEYE_ROM_ROOT}/${RELATIVE}" "mooneye;ppu" --ppu)
        endforeach()

        # DMG-only timing and OAM DMA source semantics.
        set(MOONEYE_DMG_PPU_TESTS
            "acceptance/ppu/lcdon_timing-GS.gb"
            "acceptance/ppu/lcdon_write_timing-GS.gb"
            "acceptance/oam_dma/sources-GS.gb"
        )
        foreach(RELATIVE IN LISTS MOONEYE_DMG_PPU_TESTS)
            qnemu_add_rom_test("mooneye_ppu_${RELATIVE}"
                "${MOONEYE_ROM_ROOT}/${RELATIVE}" "mooneye;ppu" --model dmg --ppu)
        endforeach()

        # Model-specific post-boot state.  The runner reproduces the boot CPU
        # registers, DIV phase and I/O values without executing a boot ROM.
        set(MOONEYE_BOOT_TESTS
            "boot_div-dmg0|dmg0"
            "boot_div-dmgABCmgb|dmgabc"
            "boot_div-S|sgb"
            "boot_div2-S|sgb2"
            "boot_hwio-dmg0|dmg0"
            "boot_hwio-dmgABCmgb|dmgabc"
            "boot_hwio-S|sgb"
            "boot_regs-dmg0|dmg0"
            "boot_regs-dmgABC|dmgabc"
            "boot_regs-mgb|mgb"
            "boot_regs-sgb|sgb"
            "boot_regs-sgb2|sgb2"
            "serial/boot_sclk_align-dmgABCmgb|dmgabc"
        )
        foreach(ENTRY IN LISTS MOONEYE_BOOT_TESTS)
            string(REPLACE "|" ";" PARTS "${ENTRY}")
            list(GET PARTS 0 RELATIVE)
            list(GET PARTS 1 BOOT_MODEL)
            qnemu_add_rom_test("mooneye_${RELATIVE}"
                "${MOONEYE_ROM_ROOT}/acceptance/${RELATIVE}.gb"
                "mooneye;boot" --model dmg --boot-model "${BOOT_MODEL}")
        endforeach()

        # CGB boot ROM execution, instead of the HLE post-boot state.
        # boot_div-* is intentionally not registered: it asserts the official
        # CGB boot ROM's exact cycle count, which SameBoy's reimplementation
        # (and this QNEMU-logo variant) does not reproduce.
        foreach(RELATIVE IN ITEMS
            "misc/boot_regs-cgb.gb"
            "misc/boot_hwio-C.gb"
        )
            qnemu_add_rom_test("mooneye_bootrom_${RELATIVE}"
                "${MOONEYE_ROM_ROOT}/${RELATIVE}"
                "mooneye;boot" --model cgb --ppu --apu --boot-rom)
        endforeach()

        # Unused I/O bits and unmapped FFxx addresses read as 1s on DMG/SGB.
        # The APU is attached so its unused-bit masks are exercised too.
        qnemu_add_rom_test("mooneye_bits_unused_hwio_gs"
            "${MOONEYE_ROM_ROOT}/acceptance/bits/unused_hwio-GS.gb"
            "mooneye;apu" --model dmg --apu)

        # The runner now uses the same GbCartridge/MBC code as the emulator.
        # MBC1M multi-cart wiring is not implemented yet, so that one is left
        # out of the curated set.
        set(MOONEYE_MBC_TESTS
            "emulator-only/mbc1/bits_bank1.gb"
            "emulator-only/mbc1/bits_bank2.gb"
            "emulator-only/mbc1/bits_mode.gb"
            "emulator-only/mbc1/bits_ramg.gb"
            "emulator-only/mbc1/multicart_rom_8Mb.gb"
            "emulator-only/mbc1/ram_256kb.gb"
            "emulator-only/mbc1/ram_64kb.gb"
            "emulator-only/mbc1/rom_16Mb.gb"
            "emulator-only/mbc1/rom_1Mb.gb"
            "emulator-only/mbc1/rom_2Mb.gb"
            "emulator-only/mbc1/rom_4Mb.gb"
            "emulator-only/mbc1/rom_512kb.gb"
            "emulator-only/mbc1/rom_8Mb.gb"
            "emulator-only/mbc2/bits_ramg.gb"
            "emulator-only/mbc2/bits_romb.gb"
            "emulator-only/mbc2/bits_unused.gb"
            "emulator-only/mbc2/ram.gb"
            "emulator-only/mbc2/rom_1Mb.gb"
            "emulator-only/mbc2/rom_2Mb.gb"
            "emulator-only/mbc2/rom_512kb.gb"
            "emulator-only/mbc5/rom_16Mb.gb"
            "emulator-only/mbc5/rom_1Mb.gb"
            "emulator-only/mbc5/rom_2Mb.gb"
            "emulator-only/mbc5/rom_32Mb.gb"
            "emulator-only/mbc5/rom_4Mb.gb"
            "emulator-only/mbc5/rom_512kb.gb"
            "emulator-only/mbc5/rom_64Mb.gb"
            "emulator-only/mbc5/rom_8Mb.gb"
        )
        foreach(RELATIVE IN LISTS MOONEYE_MBC_TESTS)
            qnemu_add_rom_test("mooneye_mbc_${RELATIVE}"
                "${MOONEYE_ROM_ROOT}/${RELATIVE}" "mooneye;mbc")
        endforeach()
    endif()

    if(NOT EXISTS "${BLARGG_ROM_ROOT}" AND NOT EXISTS "${MOONEYE_ROM_ROOT}")
        message(STATUS
            "Test ROM submodules not initialized; run "
            "'git submodule update --init --recursive' to enable ROM tests")
    endif()

    set(QNEMU_ACID2_ROM_DIR "${CMAKE_BINARY_DIR}/test-roms/acid2" CACHE PATH
        "Directory containing dmg-acid2.gb and cgb-acid2.gbc")
    set(ACID2_DMG_ROM "${QNEMU_ACID2_ROM_DIR}/dmg-acid2.gb")
    set(ACID2_CGB_ROM "${QNEMU_ACID2_ROM_DIR}/cgb-acid2.gbc")
    set(ACID2_DMG_REFERENCE "${QNEMU_TEST_ROM_ROOT}/dmg-acid2/img/reference-dmg.png")
    set(ACID2_CGB_REFERENCE "${QNEMU_TEST_ROM_ROOT}/cgb-acid2/img/reference.png")

    if(EXISTS "${ACID2_DMG_ROM}" AND EXISTS "${ACID2_DMG_REFERENCE}")
        add_test(
            NAME rom_acid2_dmg
            COMMAND gb_test_runner "${ACID2_DMG_ROM}"
                --model dmg --frames 10
                --compare-image "${ACID2_DMG_REFERENCE}")
        set_tests_properties(rom_acid2_dmg PROPERTIES
            TIMEOUT 120
            LABELS "rom;acid2;ppu")
    endif()

    if(EXISTS "${ACID2_CGB_ROM}" AND EXISTS "${ACID2_CGB_REFERENCE}")
        add_test(
            NAME rom_acid2_cgb
            COMMAND gb_test_runner "${ACID2_CGB_ROM}"
                --model cgb --frames 10
                --compare-image "${ACID2_CGB_REFERENCE}")
        set_tests_properties(rom_acid2_cgb PROPERTIES
            TIMEOUT 120
            LABELS "rom;acid2;ppu")

        add_test(
            NAME rom_acid2_cgb_boot_rom
            COMMAND gb_test_runner "${ACID2_CGB_ROM}"
                --model cgb --boot-rom --frames 240
                --compare-image "${ACID2_CGB_REFERENCE}")
        set_tests_properties(rom_acid2_cgb_boot_rom PROPERTIES
            TIMEOUT 120
            LABELS "rom;acid2;ppu;boot")
    endif()

    if(EXISTS "${QNEMU_TEST_ROM_ROOT}/dmg-acid2"
        AND NOT EXISTS "${ACID2_DMG_ROM}")
        message(STATUS
            "Acid2 ROMs not found; run "
            "'python3 tests/rom/fetch_acid2_roms.py' to enable PPU image tests")
    endif()
endif()

# Optional ad-hoc registration.  Point this at a directory tree containing
# additional .gb files; every ROM found below it is registered as a test.
set(QNEMU_TEST_ROMS_DIR "" CACHE PATH "Directory tree containing extra .gb test ROMs")
if(QNEMU_TEST_ROMS_DIR)
    file(GLOB_RECURSE QNEMU_TEST_ROMS "${QNEMU_TEST_ROMS_DIR}/*.gb")
    foreach(ROM IN LISTS QNEMU_TEST_ROMS)
        file(RELATIVE_PATH ROM_RELATIVE "${QNEMU_TEST_ROMS_DIR}" "${ROM}")
        string(MAKE_C_IDENTIFIER "rom_${ROM_RELATIVE}" TEST_NAME)
        add_test(
            NAME "${TEST_NAME}"
            COMMAND gb_test_runner "${ROM}" --max-cycles 300000000)
        set_tests_properties("${TEST_NAME}" PROPERTIES TIMEOUT 120 LABELS "rom")
    endforeach()
endif()

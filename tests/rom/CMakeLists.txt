add_executable(
    gb_test_runner
    main.cpp
    GbTestSystem.cpp)
target_include_directories(gb_test_runner PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_test_runner PRIVATE qnemu_warnings gb_cpu)

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
        COMMAND gb_test_runner "${rom_path}" --max-cycles 300000000)
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
    endif()

    set(MOONEYE_ROM_ROOT "${QNEMU_TEST_ROM_ROOT}/mooneye-test-roms")
    if(EXISTS "${MOONEYE_ROM_ROOT}/acceptance")
        # Curated subset that the CPU/timer/interrupt test bus is expected to
        # pass.  Tests depending on OAM DMA, PPU modes, APU or boot state are
        # intentionally not registered here.
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
            "acceptance/intr_timing.gb"
            "acceptance/pop_timing.gb"
            "acceptance/rapid_di_ei.gb"
            "acceptance/reti_intr_timing.gb"
            "acceptance/timer/tim00.gb"
            "acceptance/timer/tim01.gb"
            "acceptance/timer/tim10.gb"
            "acceptance/timer/tim11.gb"
            "acceptance/timer/tma_write_reloading.gb"
        )
        foreach(RELATIVE IN LISTS MOONEYE_CPU_TESTS)
            qnemu_add_rom_test("mooneye_${RELATIVE}" "${MOONEYE_ROM_ROOT}/${RELATIVE}" "mooneye")
        endforeach()
    endif()

    if(NOT EXISTS "${BLARGG_ROM_ROOT}" AND NOT EXISTS "${MOONEYE_ROM_ROOT}")
        message(STATUS
            "Test ROM submodules not initialized; run "
            "'git submodule update --init --recursive' to enable ROM tests")
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

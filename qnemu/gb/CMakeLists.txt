# The SM83 core is deliberately independent of Qt and of the rest of the
# emulator.  Keeping it in its own library makes it linkable by the headless
# ROM test runner and by CPU unit tests without dragging in the GUI.
add_library(gb_cpu STATIC ${CMAKE_CURRENT_SOURCE_DIR}/cpu/GbCpu.cpp)
set_target_properties(gb_cpu PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(gb_cpu PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_cpu PRIVATE qnemu_warnings)

# C++
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge CARTRIDGE_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge/mbc CARTRIDGE_MBC_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/interrupt INTERRUPT_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/memory MEMORY_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gpu GPU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/joypad JOYPAD_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/timer TIMER_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/mmu MMU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/apu APU_SRCS)

add_library(
    gb OBJECT
    Gb.cpp
    ${CARTRIDGE_SRCS}
    ${CARTRIDGE_MBC_SRCS}
    ${INTERRUPT_SRCS}
    ${MEMORY_SRCS}
    ${GPU_SRCS}
    ${JOYPAD_SRCS}
    ${TIMER_SRCS}
    ${MMU_SRCS}
    ${APU_SRCS}
)
target_include_directories(gb PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(gb PRIVATE qnemu_warnings gb_cpu Qt6::Core Qt6::Gui)

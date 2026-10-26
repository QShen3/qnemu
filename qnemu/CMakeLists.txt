add_subdirectory(display)
add_subdirectory(sound)
add_subdirectory(gb)

add_executable(
    qnemu
    main.cpp
    MainWindow.cpp)
target_include_directories(qnemu PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(
    qnemu
    PRIVATE
    qnemu_warnings
    display
    sound
    gb
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets)

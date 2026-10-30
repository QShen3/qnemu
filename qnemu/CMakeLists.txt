add_subdirectory(sound)
add_subdirectory(save)
add_subdirectory(gb)

add_executable(
    qnemu
    main.cpp
    AppController.cpp
    AppSettings.cpp
    EmulatorItem.cpp
    GameLibrary.cpp)
qt_add_resources(qnemu "qml"
    PREFIX "/qml"
    FILES
        Main.qml
        GameWindow.qml
        LibraryPage.qml
        SettingsDialog.qml
        Theme.qml
        SettingsCard.qml
        EmptyLibrary.qml
        StatusToast.qml)
target_include_directories(qnemu PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(
    qnemu
    PRIVATE
    qnemu_warnings
    gb
    sound
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickDialogs2)

add_subdirectory(sound)
add_subdirectory(save)
add_subdirectory(gb)

add_executable(
    qnemu
    main.cpp
    ui/AppController.cpp
    ui/AppSettings.cpp
    ui/EmulatorItem.cpp
    ui/GameLibrary.cpp
    ui/RomArchive.cpp
    ui/RomDatabase.cpp)
qt_add_resources(qnemu "qml"
    PREFIX "/qml"
    BASE ui
    FILES
        ui/Main.qml
        ui/GameWindow.qml
        ui/LibraryPage.qml
        ui/FilterComboBox.qml
        ui/SettingsDialog.qml
        ui/Theme.qml
        ui/SettingsCard.qml
        ui/EmptyLibrary.qml
        ui/StatusToast.qml)
qt_add_resources(qnemu "romdb"
    PREFIX "/data"
    BASE data
    FILES
        data/romdb.json)
target_include_directories(qnemu PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(
    qnemu
    PRIVATE
    qnemu_warnings
    gb
    sound
    ${QNEMU_LIBARCHIVE_TARGET}
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickDialogs2)

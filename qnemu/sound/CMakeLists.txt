add_library(sound STATIC DefaultSound.cpp)
set_target_properties(sound PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(sound PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(sound
    PUBLIC
        Qt6::Core
        Qt6::Multimedia
    PRIVATE qnemu_warnings)

add_library(display STATIC RasterDisplay.cpp)
set_target_properties(display PROPERTIES AUTOUIC OFF AUTORCC OFF)
target_include_directories(display PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(display
    PUBLIC Qt6::Gui
    PRIVATE qnemu_warnings)

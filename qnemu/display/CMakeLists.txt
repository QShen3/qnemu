add_library(display OBJECT RasterDisplay.cpp)
target_include_directories(display PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(display PRIVATE qnemu_warnings Qt6::Gui)

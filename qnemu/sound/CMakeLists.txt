add_library(sound OBJECT DefaultSound.cpp)
target_include_directories(sound PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(sound PRIVATE qnemu_warnings Qt6::Gui Qt6::Multimedia)

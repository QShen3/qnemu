add_library(save STATIC SaveManager.cpp)
target_include_directories(save PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(save PRIVATE qnemu_warnings Qt6::Core)

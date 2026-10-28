add_library(save STATIC SaveManager.cpp)
set_target_properties(save PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(save PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(save
    PRIVATE
        qnemu_warnings
        Qt6::Core)

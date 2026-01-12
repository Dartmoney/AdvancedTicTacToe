
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(advanced_ttt
        src/main.cpp
        src/AppConfig.h
        src/AppConfig.cpp
        src/MainWindow.h
        src/MainWindow.cpp
        src/BoardView.h
        src/BoardView.cpp
        src/GridItem.h
        src/GridItem.cpp
        src/MarkItem.h
        src/MarkItem.cpp
        src/SettingsPanel.h
        src/SettingsPanel.cpp
        src/GameOverDialog.h
        src/GameOverDialog.cpp
)

set_target_properties(advanced_ttt PROPERTIES
        AUTOMOC ON
        AUTOUIC OFF
        AUTORCC OFF
)

target_link_libraries(advanced_ttt PRIVATE
        Qt6::Widgets
        advanced_ttt_engine
)

target_include_directories(advanced_ttt PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (MSVC)
    target_compile_options(advanced_ttt PRIVATE /W4)
else()
    target_compile_options(advanced_ttt PRIVATE -Wall -Wextra -Wpedantic)
endif()

cmake_minimum_required(VERSION 3.21)

project(AdvancedTicTacToe VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ADV_TTT_BUILD_TESTS "Build engine unit tests" ON)

add_subdirectory(engine)
add_subdirectory(qt_gui)

if (ADV_TTT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

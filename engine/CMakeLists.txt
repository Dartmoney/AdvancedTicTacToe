add_library(advanced_ttt_engine
        src/FiniteBoard.cpp
        src/InfiniteBoard.cpp
        src/CellValueFunction.cpp
        src/RuleSet.cpp
        src/Scoring.cpp
        src/GameState.cpp
        src/AI.cpp
)

target_include_directories(advanced_ttt_engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(advanced_ttt_engine PUBLIC cxx_std_20)

if (MSVC)
    target_compile_options(advanced_ttt_engine PRIVATE /W4)
else()
    target_compile_options(advanced_ttt_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

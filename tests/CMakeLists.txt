add_executable(advanced_ttt_tests
        test_main.cpp
)

target_link_libraries(advanced_ttt_tests PRIVATE advanced_ttt_engine)
target_compile_features(advanced_ttt_tests PRIVATE cxx_std_20)

add_test(NAME advanced_ttt_engine_tests COMMAND advanced_ttt_tests)

cmake_minimum_required(VERSION 3.20)

# Minimal custom test runner executable
add_executable(engine_tests
    test_main.cpp
    test_math.cpp
)

target_link_libraries(engine_tests PRIVATE hyperbolic_engine)
add_test(NAME CoreEngineTests COMMAND engine_tests)

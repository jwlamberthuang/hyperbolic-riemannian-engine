cmake_minimum_required(VERSION 3.20)
project(HyperbolicRiemannianEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Aggressive optimization flags for high-performance systems engineering
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-O3 -march=native -Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(include)

# Engine Static Library Target
add_library(hyperbolic_engine STATIC
    src/engine/lorentz_math.cpp
    src/engine/trajectory_store.cpp
    src/engine/linear_scanner.cpp
    src/simulation/rgd_simulator.cpp
)

# Main Executable Application Target
add_executable(hyperbolic_app src/main.cpp)
target_link_libraries(hyperbolic_app PRIVATE hyperbolic_engine)

# Add Tests Subdirectory
enable_testing()
add_subdirectory(tests)

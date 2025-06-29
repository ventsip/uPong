# Test CMakeLists.txt for uPong
cmake_minimum_required(VERSION 3.13)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create test executable for host machine (not Pico)
project(uPong_tests)

# Include Catch2
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    # Download Catch2 if not found
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mocks)

# Source files to test (extract game logic without hardware dependencies)
set(GAME_SOURCES
    game_logic.cpp
)

# Mock implementations
set(MOCK_SOURCES
    mocks/ws2812_mock.cpp
    mocks/screen_mock.cpp
    mocks/screen_primitives_mock.cpp
    mocks/rotary_encoder_mock.cpp
)

# Test source files
set(TEST_SOURCES
    unit/test_main.cpp
    unit/test_point_vector.cpp
    unit/test_movable_point.cpp
    unit/test_collision_detection.cpp
)

# Create test executable
add_executable(uPong_tests
    ${GAME_SOURCES}
    ${MOCK_SOURCES}
    ${TEST_SOURCES}
)

# Link with Catch2
target_link_libraries(uPong_tests PRIVATE Catch2::Catch2WithMain)

# Enable testing
enable_testing()
add_test(NAME uPong_unit_tests COMMAND uPong_tests)

# Compiler flags for host build
target_compile_options(uPong_tests PRIVATE -Wall -Wextra -g)

# Define HOST_BUILD to conditionally compile host-specific code
target_compile_definitions(uPong_tests PRIVATE HOST_BUILD=1)
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(uPong C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(uPong src/uPong.cpp src/ws2812.cpp src/screen.cpp src/pong_game.cpp src/rotary_encoder.cpp)

pico_set_program_name(uPong "uPong")
pico_set_program_version(uPong "0.1")

# Generate PIO header
pico_generate_pio_header(uPong ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_usb(uPong 1)
pico_enable_stdio_uart(uPong 0)

# Add the standard library to the build
target_link_libraries(uPong
        pico_stdlib
        pico_multicore
        )

# Set compiler flags for highest warning level and treat warnings as errors
target_compile_options(uPong PRIVATE -Wall -Wextra -Werror)

# Add the standard include files to the build
target_include_directories(uPong PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(uPong 
        hardware_dma
        hardware_pio
        )

pico_add_extra_outputs(uPong)

# Add testing support
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()


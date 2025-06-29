# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

uPong is a C++ embedded systems project implementing a Pong game for Raspberry Pi Pico 2. The game runs on a 48x32 pixel WS2812 LED matrix display with dual rotary encoder controls.

## Build and Development Commands

### Build
```bash
mkdir build
cd build
cmake ..
make
```

### Flash to Pico
```bash
# Hold BOOTSEL button on Pico while plugging in
cp uPong.uf2 /Volumes/RPI-RP2  # macOS
# cp uPong.uf2 /media/*/RPI-RP2  # Linux
```

### Clean Build
```bash
rm -rf build
```

## Architecture

### Core Components
- **uPong.cpp**: Main entry point with game loop and FPS tracking
- **pong_game.cpp/hpp**: Game logic with object-oriented design (CPoint, CVector, CMovablePoint classes)
- **ws2812.cpp/hpp**: WS2812 LED driver with PIO assembly code
- **screen.cpp/hpp**: Display buffer management and rendering pipeline
- **rotary_encoder.cpp/hpp**: Player input handling

### Hardware Abstraction
- **Display**: 48x32 pixels (3x2 grid of 16x16 LED matrices)
- **Input**: Dual rotary encoders for two-player control
- **Processing**: Dual-core ARM Cortex-M33 (game logic on core 0, rendering on core 1)

### Key Design Patterns
- Object-oriented game entities with physics (position, velocity, movement)
- Hardware abstraction layer separating game logic from display/input
- DMA-accelerated LED data transmission
- Real-time performance profiling with microsecond precision

## Development Notes

### Compiler Configuration
- C++17 standard with strict warnings (-Wall -Wextra -Werror)
- Raspberry Pi Pico SDK 2.1.0
- Target board: pico2

### Performance Considerations
- Frame rate monitoring built into main loop
- Rendering pipeline optimized with clipping and alpha blending
- Multi-core utilization for concurrent game logic and display updates

### Testing
- Visual test patterns available in uPong_tests.h
- Built-in diagnostics and performance measurements
- Unit tests available for host development (see Testing section below)

## Testing

### Unit Tests (Host Testing)
The project includes comprehensive unit tests that run on the host machine (macOS/Linux) without requiring Pico hardware.

#### Build and Run Tests
```bash
# Create test build directory
mkdir build-tests
cd build-tests

# Configure for host testing
cmake .. -DBUILD_TESTING=ON

# Build tests
make

# Run tests
./uPong_tests
```

#### Test Coverage
- **CPoint/CVector**: Vector math, rotation, scalar operations
- **CMovablePoint**: Physics simulation and movement
- **Collision Detection**: Paddle-ball collision logic
- **Game Logic**: Ball bouncing, scoring, field boundaries

#### Test Architecture
- **Catch2**: Modern C++ testing framework
- **Hardware Mocks**: Mock implementations of WS2812, screen, and rotary encoder interfaces
- **Isolated Logic**: Game classes extracted for testing without hardware dependencies

#### Adding New Tests
1. Create test files in `tests/unit/`
2. Include `game_logic.hpp` for testable classes
3. Use Catch2 macros (`TEST_CASE`, `REQUIRE`, `Catch::Approx`)
4. Update `tests/CMakeLists.txt` if needed
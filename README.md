# uPong

uController-based Pong Game for Raspberry Pi Pico 2

A C++ embedded Pong game running on a 48x32 pixel WS2812 LED matrix with dual rotary encoder controls.

## Hardware Requirements

- Raspberry Pi Pico 2
- 48x32 WS2812 LED matrix (3x2 grid of 16x16 matrices)
- 2x Rotary encoders for player controls

## Building for Pico

### Prerequisites

- Raspberry Pi Pico SDK 2.1.0
- CMake 3.13+
- ARM GCC toolchain

### Build Steps

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

## Development & Testing

### Unit Tests (Host Development)

Run comprehensive tests on your development machine without Pico hardware:

```bash
# Navigate to tests directory
cd tests

# Create build directory and configure
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

# Build and run tests
make && ./uPong_tests
```

**Test Coverage:**

- Vector math and rotation operations
- Physics simulation and movement
- Collision detection algorithms
- Game logic validation

**Expected Output:**

```
===============================================================================
All tests passed (87 assertions in 4 test cases)
```

### Architecture

- **Object-oriented design**: CPoint, CVector, CMovablePoint classes
- **Hardware abstraction**: Separate game logic from display/input
- **Real-time performance**: Dual-core processing with FPS monitoring
- **Optimized rendering**: DMA-accelerated LED transmission

## Project Structure

```
src/
├── uPong.cpp           # Main game loop
├── pong_game.cpp       # Game logic
├── ws2812.cpp          # LED matrix driver
├── screen.cpp          # Display management
└── rotary_encoder.cpp  # Input handling

tests/
├── unit/               # Unit test suites
├── mocks/              # Hardware mocks
└── game_logic.hpp      # Testable game classes
```

## License

[Add your license here]

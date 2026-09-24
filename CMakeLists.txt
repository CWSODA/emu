cmake_minimum_required(VERSION 3.20)  # CMake version check
project(emu)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

# Add main.cpp file of the project root directory as a source file
set(SOURCE_FILES 
    main.cpp 
    cpu.cpp 
    cpu_timer.cpp
    opcode_instructions.cpp
    interrupts.cpp
    file_reader.cpp
    mem.cpp
    graphics/ppu.cpp
)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(emu ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.20)  # CMake version check
project(emu)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 20)            # Enable c++14 standard

# Add main.cpp file of the project root directory as a source file
file(GLOB_RECURSE source_list "src/*.cpp")
set(INCLUDE_DIR 
    "src"
    "src/cpu"
    "src/graphics"
)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(emu ${source_list})
target_include_directories(emu PRIVATE ${INCLUDE_DIR})
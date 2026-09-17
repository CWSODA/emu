#pragma once

#include <fstream>
#include <cstdint>
#include <vector>

class ROM {
   public:
    ROM(const char* filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot read file!");

        data = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
    }

    std::vector<uint8_t> data;
};

inline void load_ROM_from_path(const char* filename, uint8_t* memory) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot read ROM file!");

    file.seekg(0, file.end);  // go to end of file
    size_t ROM_size = file.tellg();

    file.seekg(0, file.beg);  // back to start
    file.read((char*)memory, ROM_size);
}
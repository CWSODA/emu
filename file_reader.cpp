#include <fstream>
#include <cstdint>
#include <vector>

#include "data.hpp"

void Data::load_ROM_from_path(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot read ROM file!");

    file.seekg(0, file.end);  // go to end of file
    size_t ROM_size = file.tellg();

    file.seekg(0, file.beg);  // back to start
    file.read((char*)memory, ROM_size);
}

const char* test_roms[] = {
    "../test_cpu/cpu_instrs.gb",
    "../test_cpu/individual/01-special.gb",
    "../test_cpu/individual/02-interrupts.gb",
    "../test_cpu/individual/03-op sp,hl.gb",
    "../test_cpu/individual/04-op r,imm.gb",
    "../test_cpu/individual/05-op rp.gb",
    "../test_cpu/individual/06-ld r,r.gb",
    "../test_cpu/individual/07-jr,jp,call,ret,rst.gb",
    "../test_cpu/individual/08-misc instrs.gb",
    "../test_cpu/individual/09-op r,r.gb",
    "../test_cpu/individual/10-bit ops.gb",
    "../test_cpu/individual/11-op a,(hl).gb",
};
void Data::load_test_ROM(uint8_t idx) {
    auto filename = test_roms[idx];
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot read ROM file!");

    file.seekg(0, file.end);  // go to end of file
    size_t ROM_size = file.tellg();

    file.seekg(0, file.beg);  // back to start
    file.read((char*)memory, ROM_size);
}
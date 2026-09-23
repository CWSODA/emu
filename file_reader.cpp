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
    "../gb-test-roms-master/cpu_instrs/cpu_instrs.gb",
    "../gb-test-roms-master/cpu_instrs/individual/01-special.gb",
    "../gb-test-roms-master/cpu_instrs/individual/02-interrupts.gb",
    "../gb-test-roms-master/cpu_instrs/individual/03-op sp,hl.gb",
    "../gb-test-roms-master/cpu_instrs/individual/04-op r,imm.gb",
    "../gb-test-roms-master/cpu_instrs/individual/05-op rp.gb",
    "../gb-test-roms-master/cpu_instrs/individual/06-ld r,r.gb",
    "../gb-test-roms-master/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb",
    "../gb-test-roms-master/cpu_instrs/individual/08-misc instrs.gb",
    "../gb-test-roms-master/cpu_instrs/individual/09-op r,r.gb",
    "../gb-test-roms-master/cpu_instrs/individual/10-bit ops.gb",
    "../gb-test-roms-master/cpu_instrs/individual/11-op a,(hl).gb",
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
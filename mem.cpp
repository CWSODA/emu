#include "data.hpp"
#include "logger.hpp"

uint8_t Data::read_mem(uint16_t addr) {
    log() << "read mem: " << std::hex << "0x" << addr << std::endl;
    return memory[addr];
}

void Data::set_mem(uint16_t addr, uint8_t val) {
    log() << "set mem: " << std::hex << "0x" << addr << std::endl;
    memory[addr] = val;
}

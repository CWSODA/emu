#include "data.hpp"
#include "logger.hpp"

uint8_t Data::read_mem(uint16_t addr) {
    if (addr == 0xff01 || addr == 0xff02) {
        printf("wrote to addr: 0x%02X\n", addr);
    }
    LOG_MEM_LINE("read mem: 0x" << addr);
    return memory[addr];
}

void Data::set_mem(uint16_t addr, uint8_t val) {
    if (addr == 0xff01 || addr == 0xff02) {
        printf("wrote to addr: 0x%02X\n", addr);
        LOG_LINE(static_cast<char>(val));
    }
    LOG_MEM_LINE("set mem: 0x" << addr);
    memory[addr] = val;
}

#include "data.hpp"
#include "logger.hpp"

uint8_t Data::read_mem(uint16_t addr) {
    LOG_MEM_LINE("read mem: 0x" << addr);
    return memory[addr];
}

void Data::set_mem(uint16_t addr, uint8_t val) {
    if (addr == 0xff01) {
        char c = static_cast<char>(val);
        log() << c;
        putc(c, stdout);
    }
    LOG_MEM_LINE("set mem: 0x" << addr);
    memory[addr] = val;
}

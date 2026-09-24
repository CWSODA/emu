#include "memory.hpp"
#include "logger.hpp"
#include "graphics/ppu.hpp"

uint8_t MemoryManager::read_mem(uint16_t addr) {
    LOG_MEM_LINE(std::setfill('0') << "read mem: 0x" << std::setw(4) << addr << "| 0x"
                                   << std::setw(2) << (int)memory[addr]);
    if (addr == 0xff41) {
        puts("stat");
    }
    return memory[addr];
}

void MemoryManager::set_mem(uint16_t addr, uint8_t val) {
    if (addr == 0xff01) {
        char c = static_cast<char>(val);
        log() << c;
        putc(c, stdout);
    } else if (addr == DIV_ADDR) {  // timer div reg, all writes reset
        memory[addr] = 0x00;
        return;
    } else if (addr == TAC_ADDR) {  // set clock divider
        switch (val & 0b11) {
            case 0b00: {
                timer->clock_divider = 256;
                break;
            }
            case 0b01: {
                timer->clock_divider = 4;
                break;
            }
            case 0b10: {
                timer->clock_divider = 16;
                break;
            }
            case 0b11: {
                timer->clock_divider = 64;
                break;
            }
        }
    } else if (addr == 0xff46) {  // DMA copy to OAM, 160 cycles
        // copy from 0xXX00-0xXX9f to 0xfe00-0xfe9f
        memcpy(&memory[0xfe00], &memory[val * 0x100], 0x9f + 1);
        puts("DMA copy");
    } else if (addr == 0xff40) {  // LCD control register
        // std::cout << "LCDC: " << cvt_binary(val) << '\n';
        ppu->set_config(val);
        ppu->background();
        ppu->print_VRAM();
    } else if (addr == 0xff45) {
        ppu->LYC = val;
    } else if (addr >= 0xfe00 && addr <= 0xfe9f) {
        puts("writing to OAM");
    }
    LOG_MEM_LINE("set mem: 0x" << addr);
    memory[addr] = val;
}

void MemoryManager::dump_mem() {
    auto file = std::ofstream("../logs/mem_dump");
    for (int x = 0; x < 0xffff + 1; x++) {
        file << memory[x];
    }
    file.close();
}

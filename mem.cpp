#include "data.hpp"
#include "logger.hpp"
#include "graphics/ppu.hpp"

constexpr uint16_t DIV_ADDR = 0xff04;   // div register
constexpr uint16_t TIMA_ADDR = 0xff05;  // timer counter
constexpr uint16_t TMA_ADDR = 0xff06;   // timer modulo
constexpr uint16_t TAC_ADDR = 0xff07;   // timer control

uint8_t Data::read_mem(uint16_t addr) {
    LOG_MEM_LINE("read mem: 0x" << addr);
    if (addr == 0xff41) {
        puts("stat");
    }
    return memory[addr];
}

void Data::set_mem(uint16_t addr, uint8_t val) {
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
                clock_divider = 256;
                break;
            }
            case 0b01: {
                clock_divider = 4;
                break;
            }
            case 0b10: {
                clock_divider = 16;
                break;
            }
            case 0b11: {
                clock_divider = 64;
                break;
            }
        }
    } else if (addr == 0xff46) {  // DMA copy to OAM, 160 cycles
        // copy from 0xXX00-0xXX9f to 0xfe00-0xfe9f
        memcpy(&memory[0xfe00], &memory[val * 0x100], 0x9f + 1);
        puts("DMA copy");
    } else if (addr == 0xff40) {  // LCD control register
        printf("Setting LCDC to %s\n", cvt_binary(val).c_str());
        if (val & 0b1000'0000) {
            dump_mem();  // LCB enabled
            static PPU ppu(memory);
            ppu.start_frame();
            ppu.print_VRAM();
        }
    } else if (addr == 0xff45) {
        puts("changing compare");
    } else if (addr >= 0xfe00 && addr <= 0xfe9f) {
        puts("writing to OAM");
    }
    LOG_MEM_LINE("set mem: 0x" << addr);
    memory[addr] = val;
}

void Data::inc_DIV() { memory[DIV_ADDR] += 1; }
void Data::inc_TIMA() {
    // check for TIMA overflow
    if (memory[TIMA_ADDR] == 0xff) {
        // req timer interrupt (bit 2)
        memory[IF_ADDR] = memory[IF_ADDR] | (1 << 2);

        // technically this is set one cycle after, as in there is one cycle when TIMA is 0x00
        memory[TIMA_ADDR] = memory[TMA_ADDR];
    } else {
        memory[TIMA_ADDR]++;
    }
}

void Data::dump_mem() {
    auto file = std::ofstream("../logs/mem_dump");
    for (int x = 0; x < 0xffff + 1; x++) {
        file << memory[x];
    }
    file.close();
}

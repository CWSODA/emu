#pragma once
#include "ppu.hpp"
#include "timer.hpp"

class MemoryManager {
   public:
    MemoryManager() {}
    void init(uint8_t* memory, PPU* ppu, Timer* timer) {
        this->memory = memory;
        this->ppu = ppu;
        this->timer = timer;
    }
    uint8_t read_mem(uint16_t addr);
    void set_mem(uint16_t addr, uint8_t val);
    void dump_mem();

   private:
    uint8_t* memory;

    // access to other components
    PPU* ppu;
    Timer* timer;
};
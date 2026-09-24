#pragma once
#include <stdint.h>
#include "ppu.hpp"
#include "timer.hpp"

class Clock {
   public:
    void init(PPU* ppu, Timer* timer) {
        this->ppu = ppu;
        this->timer = timer;
    }
    void tick(uint8_t cycles) {
        ppu->tick(cycles);
        ;
    }

   private:
    PPU* ppu;
    Timer* timer;
};
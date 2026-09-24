#pragma once
#include <stdint.h>

class Timer {
   public:
    void init(uint8_t* memory) { this->memory = memory; }
    void tick(uint8_t cycles);
    uint32_t clock_divider = 1024;  // default

   private:
    uint8_t* memory;
    uint32_t tima_ticks = 0;

    void inc_tima();
    void inc_div();
};
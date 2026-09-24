#include "timer.hpp"
#include "memory_addr.hpp"

// tac modes:
// TAC = 00 → 4096 Hz  → 256 CPU cycles
// TAC = 01 → 262144 Hz → 4 CPU cycles
// TAC = 10 → 65536 Hz  → 16 CPU cycles
// TAC = 11 → 16384 Hz  → 64 CPU cycles

void Timer::tick(uint8_t cycles) {
    static uint32_t div_ticks = 0;
    div_ticks += cycles;
    if (div_ticks >= 256) {
        div_ticks -= 256;
        inc_div();
    }

    uint8_t tac_val = memory[TAC_ADDR];
    if (!(tac_val & 0b100)) return;
    tima_ticks += cycles;

    if (tima_ticks >= clock_divider) {
        tima_ticks -= clock_divider;
        inc_tima();
    }
}

void Timer::inc_div() { memory[DIV_ADDR] += 1; }
void Timer::inc_tima() {
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

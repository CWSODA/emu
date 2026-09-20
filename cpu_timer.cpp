#include "cpu.hpp"

constexpr uint16_t DIV = 0xff04;   // div register
constexpr uint16_t TIMA = 0xff05;  // timer counter
constexpr uint16_t TMA = 0xff06;   // timer modulo
constexpr uint16_t TAC = 0xff07;   // timer control

// tac modes:
// TAC = 00 → 4096 Hz  → 256 CPU cycles
// TAC = 01 → 262144 Hz → 4 CPU cycles
// TAC = 10 → 65536 Hz  → 16 CPU cycles
// TAC = 11 → 16384 Hz  → 64 CPU cycles

void CPU::tick_timer(clock_cycles cycles) {
    static uint32_t div_ticks = 0;
    div_ticks += cycles;
    if (div_ticks >= 256) {
        div_ticks -= 256;
        data.inc_DIV();
    }

    static uint32_t tima_ticks = 0;
    uint8_t tac_val = data.read_mem(TAC);
    if (!(tac_val & 0b100)) return;
    tima_ticks += cycles;
    uint32_t clock_divider = data.get_clock_divider();
    if (tima_ticks >= clock_divider) {
        tima_ticks -= clock_divider;
        data.inc_TIMA();
    }
}

void CPU::inc_tima() {
    uint8_t tima = data.read_mem(TIMA);
    if (tima == 0xff) {
        // req timer interrupt
        data.set_mem(0xff0f, data.read_mem(0xff0f) | (1 << 2));
        data.set_mem(TIMA, data.read_mem(TMA));
    } else {
        data.inc_TIMA();
    }
}

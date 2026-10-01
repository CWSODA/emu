#pragma once
#include "cpu.hpp"
#include "memory.hpp"
#include "ppu.hpp"
#include "timer.hpp"
#include "clock.hpp"

#include "logger.hpp"

class GameBoy {
   public:
    GameBoy() {
        ppu.init(memory);
        timer.init(memory);
        cpu.init(&mm, &clock);
        clock.init(&ppu, &timer);
        mm.init(memory, &ppu, &timer);
    }
    void load_test_rom(uint8_t idx);
    void load_rom_from_path(const char* filename);
    int run() { return cpu.run(); }
    int run_n_cycles(int cycles) { return cpu.run_n_cycles(cycles); }
    bool check_frame_ready() { return ppu.check_frame_ready(); }
    const uint8_t* get_frame_data() { return ppu.get_frame_data(); }

    /* ------------------------ dump ------------------------ */
    void dump_cpu_state() { cpu.dump_state(log_misc(), true); }
    void dump_memory() {
        auto file = std::ofstream("../logs/mem_dump");
        for (int x = 0; x < 0xffff + 1; x++) {
            file << memory[x];
        }
        file.close();
    }

   private:
    uint8_t memory[0xff'ff + 1] = {};
    MemoryManager mm;

    CPU cpu;
    PPU ppu;
    Timer timer;
    Clock clock;
};
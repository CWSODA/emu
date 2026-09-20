#include "cpu.hpp"
#include "logger.hpp"

// TESTS PASSED SO FAR ARE HIGHLIGHTED:
// 00 - all
/* -------------------- 01 - special -------------------- */
// 02 - interrupts
// 03 - op sp, hl
// 04 - op r, imm
/* --------------------- 05 - op rp --------------------- */
/* -------------------- 06 - ld r, r -------------------- */
/* ------------- 07 - jr, jp, call, ret, rst ------------ */
/* --------------- 08 - misc instructions --------------- */
// 09 - op r, r
/* -------------------- 10 - bit ops -------------------- */
// 11 - op a, [hl]

int main(int argc, char* argv[]) {
    CPU cpu;
    if (argc == 2) {  // loads rom if argument given
        cpu.load_rom(argv[1]);
    } else if (argc == 3) {  // loads test rom
        int test_rom_idx = std::stoi(argv[2]);
        printf("Loading test rom(%d)...\n", test_rom_idx);
        cpu.load_test_rom(test_rom_idx);
    }

    while (cpu.run()) {
        cpu.dump_state(log_misc(), true);
    }

    return 0;
}
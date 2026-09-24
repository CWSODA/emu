#include "gameboy.hpp"
#include "logger.hpp"

int main(int argc, char* argv[]) {
    GameBoy gb;
    if (argc == 2) {  // loads rom if argument given
        gb.load_rom_from_path(argv[1]);
    } else if (argc == 3) {  // loads test rom
        int test_rom_idx = std::stoi(argv[2]);
        printf("Loading test rom(%d)...\n", test_rom_idx);
        gb.load_test_rom(test_rom_idx);
    }
    gb.dump_memory();
    while (gb.run()) {
        gb.dump_cpu_state();
    }

    return 0;
}
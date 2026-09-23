#include "cpu.hpp"
#include "logger.hpp"

int main(int argc, char* argv[]) {
    CPU cpu;
    if (argc == 2) {  // loads rom if argument given
        cpu.load_rom(argv[1]);
    } else if (argc == 3) {  // loads test rom
        int test_rom_idx = std::stoi(argv[2]);
        printf("Loading test rom(%d)...\n", test_rom_idx);
        cpu.load_test_rom(test_rom_idx);
    }

    while (cpu.run(false)) {
        cpu.dump_state(log_misc(), true);
    }

    return 0;
}
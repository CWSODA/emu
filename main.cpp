#include "cpu.hpp"
#include "logger.hpp"

int main(int argc, char* argv[]) {
    CPU cpu;
    if (argc == 2) {  // loads rom if argument given
        cpu.load_rom(argv[1]);
    } else {  // loads test rom
        puts("Loading test rom...");
        cpu.load_test_rom(1);
    }
    cpu.dump_state(log_misc());

    while (cpu.run()) {
        // cpu.dump_state(log_misc());
    }
    cpu.dump_state(log_misc());

    return 0;
}
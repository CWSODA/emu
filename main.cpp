#include "cpu.hpp"
#include "logger.hpp"

int main() {
    CPU cpu;
    cpu.load_test_rom(1);
    cpu.dump_state(log_misc());
    cpu.parse_byte(0b0);
    cpu.dump_state(log_misc());

    while (cpu.run());

    return 0;
}
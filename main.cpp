#include "cpu.hpp"

int main() {
    CPU cpu;
    cpu.load_ROM("../test_cpu/individual/07-jr,jp,call,ret,rst.gb");

    while (cpu.run());

    return 0;
}
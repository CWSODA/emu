#include "cpu.hpp"

int main() {
    CPU cpu;
    cpu.load_ROM("../test_cpu/individual/07-jr,jp,call,ret,rst.gb");

    int count = 0;
    while (true) {
        cpu.run();
        if (++count >= 5 * 10000) break;
    }

    return 0;
}
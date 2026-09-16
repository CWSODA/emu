#include "cpu.hpp"

#include "file_reader.hpp"

int main() {
    CPU cpu;
    ROM rom("../test_cpu/individual/07-jr,jp,call,ret,rst.gb");

    int count = 0;
    while (true) {
        cpu.run(rom.data);
        if (++count >= 10) break;
    }

    return 0;
}
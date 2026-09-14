#include "cpu.hpp"

uint8_t CPU::calc_add8(uint8_t a, uint8_t b) {
    flags.n = false;
    flags.c = a > (0xff - b);
    flags.h = (a & 0x0f + b & 0x0f) > 0x0f;

    uint8_t result = a + b;
    flags.z = (result == 0);
    return result;
}
uint8_t CPU::calc_adc8(uint8_t a, uint8_t b) {
    flags.n = false;
    flags.h = (a & 0x0f + b & 0x0f + flags.c) > 0x0f;

    uint8_t result = a + b + flags.c;
    flags.z = (result == 0);
    flags.c = (static_cast<uint16_t>(a) + b + flags.c) > 0xff;
    return result;
}
uint8_t CPU::calc_sub8(uint8_t a, uint8_t b) {
    flags.n = true;
    flags.c = a < b;                    // underflow
    flags.h = (a & 0x0f) < (b & 0x0f);  // borrow from bit4

    uint8_t result = a - b;
    flags.z = (result == 0);
    return result;
}
uint8_t CPU::calc_sbc8(uint8_t a, uint8_t b) {
    flags.n = true;
    flags.h = (a & 0x0f) < (b & 0x0f + 1);  // borrow from bit4

    uint8_t result = a - b - flags.c;
    flags.z = (result == 0);
    flags.c = a < (static_cast<uint16_t>(b) + 1);  // underflow
    return result;
}
uint8_t CPU::calc_and8(uint8_t a, uint8_t b) {
    uint8_t result = a & b;
    set_flags_znhc(result == 0, false, true, false);
    return result;
}
uint8_t CPU::calc_xor8(uint8_t a, uint8_t b) {
    uint8_t result = a ^ b;
    set_flags_znhc(result == 0, false, false, false);
    return result;
}
uint8_t CPU::calc_or8(uint8_t a, uint8_t b) {
    uint8_t result = a | b;
    set_flags_znhc(result == 0, false, false, false);
    return result;
}
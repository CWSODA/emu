#include <stdlib.h>

// 16-bit registers
uint8_t reg8[8];  // b, c, d, e, h, l, [hl], a registers
// references for convinience
uint8_t& reg_a = reg8[7];
uint16_t SP;
uint16_t PC = 0;

uint8_t& r8(uint8_t idx) {
    if (idx == 6) {
    }
    return reg8[idx];
}

// uint16_t AF;
// uint16_t BC;
// uint16_t DE;
// uint16_t HL;
// bc, de, hl, sp
uint16_t get_r16(uint8_t idx) {
    switch (idx) {
        case 0:
            return (uint16_t)reg8[0] << 8 + reg8[1];
        case 1:
            return (uint16_t)reg8[2] << 8 + reg8[3];
        case 2:
            return (uint16_t)reg8[4] << 8 + reg8[5];
        case 3:
            return SP;
    }
}
// bc, de, hl, sp
void set_r16(uint8_t idx, uint16_t val) {
    switch (idx) {
        case 0:
            reg8[0] = val >> 8;  // MSB
            reg8[1] = val >> 8;  // LSB
            break;
        case 1:
            reg8[2] = val >> 8;  // MSB
            reg8[3] = val >> 8;  // LSB
            break;
        case 2:
            reg8[4] = val >> 8;  // MSB
            reg8[5] = val >> 8;  // LSB
            break;
        case 3:
            SP = val;
            break;
    }
}
uint16_t get_hl() { return reg8[4] << 8 + reg8[5]; }
void set_hl(uint16_t val) {
    reg8[4] = val >> 8;
    reg8[5] = val;
}

uint16_t get_bc() { return reg8[0] << 8 + reg8[1]; }
uint16_t get_de() { return reg8[2] << 8 + reg8[3]; }
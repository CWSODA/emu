#include "cpu_data.hpp"

// returns register values in order: B, C, D, E, H, L, [HL], A
// [hl] is a dummy register. use CPU get_r8/set_r8 for access
uint8_t& CPUData::r8(uint8_t idx) { return reg8[idx]; }

// registers in order: bc, de, hl, sp
uint16_t CPUData::get_r16(uint8_t idx) {
    switch (idx) {
        case 0:
            return ((uint16_t)reg8[0] << 8) + reg8[1];
        case 1:
            return ((uint16_t)reg8[2] << 8) + reg8[3];
        case 2:
            return ((uint16_t)reg8[4] << 8) + reg8[5];
        case 3:
            return SP;
    }
    printf("Invalid r16 index: %02X\n", idx);
    return 0;
}

// registers in order: bc, de, hl, sp
void CPUData::set_r16(uint8_t idx, uint16_t val) {
    switch (idx) {
        case 0:
            reg8[0] = val >> 8;    // MSB
            reg8[1] = val & 0xff;  // LSB
            break;
        case 1:
            reg8[2] = val >> 8;    // MSB
            reg8[3] = val & 0xff;  // LSB
            break;
        case 2:
            reg8[4] = val >> 8;    // MSB
            reg8[5] = val & 0xff;  // LSB
            break;
        case 3:
            SP = val;
            break;
    }
}
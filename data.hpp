#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>

constexpr uint16_t IF_ADDR = 0xff0f;  // interrupt req
constexpr uint16_t IE_ADDR = 0xffff;  // interrupt enabled

// stores and handles register/memory data
struct Data {
    Data() {}

    // 16-bit
    uint16_t SP = 0xfffe;

    uint8_t& a() { return reg8[6]; }
    uint8_t& c() { return reg8[1]; }

    // returns register values in order: B, C, D, E, H, L, [HL], A
    uint8_t& r8(uint8_t idx) {
        if (idx == 6) {
            return memory[get_hl()];
        }
        if (idx == 7) return a();
        return reg8[idx];
    }

    // registers in order: bc, de, hl, sp
    uint16_t get_r16(uint8_t idx) {
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
    void set_r16(uint8_t idx, uint16_t val) {
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

    uint16_t get_hl() { return (reg8[4] << 8) + reg8[5]; }
    void set_hl(uint16_t val) {
        reg8[4] = val >> 8;
        reg8[5] = (val & 0xff);
    }
    uint16_t get_bc() { return (reg8[0] << 8) + reg8[1]; }
    uint16_t get_de() { return (reg8[2] << 8) + reg8[3]; }

    uint16_t get_PC() { return PC; }
    void set_PC(uint16_t addr) {
        PC = addr;
        // printf("Setting PC to: 0x%04X\n", PC);
    }
    void signed_offset_PC(uint8_t byte) { set_PC(PC + (int8_t)byte); }
    void inc_PC() { PC++; }

    uint8_t read_mem(uint16_t addr);
    void set_mem(uint16_t addr, uint8_t val);
    void load_ROM_from_path(const char* filename);
    void load_test_ROM(uint8_t idx);
    void inc_DIV();
    void inc_TIMA();
    uint32_t get_clock_divider() { return clock_divider; };

   private:
    // 8-bit registers
    uint8_t reg8[7];  // b, c, d, e, h, l, a registers
    uint16_t PC = 0x0100;
    uint8_t memory[0xff'ff + 1] = {};
    uint32_t clock_divider = 1024;  // default
};

inline std::string cvt_binary(uint8_t byte) {
    std::string out(8, '0');
    for (int x = 0; x < 8; x++) {
        out.at(7 - x) = (byte & (1 << x)) ? '1' : '0';
    }
    return out;
}
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>

#include "memory_addr.hpp"

// stores and handles register/memory data
struct CPUData {
    // 16-bit
    uint16_t SP = 0xfffe;

    uint8_t& a() { return reg8[7]; }
    uint8_t& c() { return reg8[1]; }

    uint8_t& r8(uint8_t idx);                 // B, C, D, E, H, L, [HL], A
    uint16_t get_r16(uint8_t idx);            // registers in order: bc, de, hl, sp
    void set_r16(uint8_t idx, uint16_t val);  // registers in order: bc, de, hl, sp

    uint16_t get_hl() { return (reg8[4] << 8) + reg8[5]; }
    void set_hl(uint16_t val) {
        reg8[4] = val >> 8;
        reg8[5] = (val & 0xff);
    }
    uint16_t get_bc() { return (reg8[0] << 8) + reg8[1]; }
    uint16_t get_de() { return (reg8[2] << 8) + reg8[3]; }

    uint16_t get_PC() { return PC; }
    void set_PC(uint16_t addr) { PC = addr; }
    void signed_offset_PC(uint8_t byte) { set_PC(PC + (int8_t)byte); }
    void inc_PC() { PC++; }

    void load_ROM_from_path(const char* filename);
    void load_test_ROM(uint8_t idx);

   private:
    uint8_t reg8[8];  // b, c, d, e, h, l, dummy [hl], a registers
    uint16_t PC = PC_START_ADDR;
};
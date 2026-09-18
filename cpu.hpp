#pragma once

#include <stdlib.h>
#include <vector>
#include <iostream>
#include <iomanip>

#include "data.hpp"
#include "opcodes.hpp"

#define DEBUG_OPCODE
#ifdef DEBUG_OPCODE
#define CODE(...) printf(">>> %s\n", __VA_ARGS__)
#else
#define CODE(...)
#endif

// F = flags register
struct Flags {
    bool z;  // zero result
    bool n;  // subtract
    bool h;  // half carry
    bool c;  // carry, when addition overflows or subtraction underflows, or when shifting out 1
};

typedef uint8_t clock_cycles;
constexpr size_t SOME_LARGE_NUMBER = 5e3;
class CPU {
   public:
    CPU() {}
    void load_ROM(std::string path) { data.load_ROM_from_path(path.c_str()); }
    bool run();
    clock_cycles parse_byte(uint8_t byte);

   private:
    clock_cycles parse_opcode();
    clock_cycles parse_block0();
    clock_cycles parse_block3();
    void parse_8bit_arith();

    // extra data handling
    clock_cycles handle_imm16();
    clock_cycles handle_imm8(uint8_t byte);
    clock_cycles handle_cb(uint8_t byte);

    // flags and mem
    Flags flags;
    void set_flags_znhc(bool z, bool n, bool h, bool c) {
        flags.z = z;
        flags.n = n;
        flags.h = h;
        flags.c = c;
    }
    void set_flags_nhc(bool n, bool h, bool c) {
        flags.n = n;
        flags.h = h;
        flags.c = c;
    }

    /* ---------- 8-bit arithmetics and sets flags ---------- */
    uint8_t calc_add8(uint8_t a, uint8_t b);
    uint8_t calc_adc8(uint8_t a, uint8_t b);
    uint8_t calc_sub8(uint8_t a, uint8_t b);  // use for cp8 as well
    uint8_t calc_sbc8(uint8_t a, uint8_t b);
    uint8_t calc_and8(uint8_t a, uint8_t b);
    uint8_t calc_xor8(uint8_t a, uint8_t b);
    uint8_t calc_or8(uint8_t a, uint8_t b);

    uint8_t opcode;
    OpState op_state = OpState::READY;
    uint16_t imm16;
    uint8_t r16_addr;  // for imm16 stuff
    uint8_t r8_addr;   // for imm8 stuff, also used for jr cond

    Data data;
    uint16_t read_mem16(uint16_t addr) {
        return data.read_mem(addr) + (data.read_mem(addr + 1) << 8);
    }
    void ret() {
        data.set_PC(read_mem16(data.SP));
        data.SP += 2;
    }
    void call(uint16_t addr) {  // store data.PC in data.SP and jump to addr
        uint16_t ret_addr = data.get_PC() + 1;
        data.set_mem(--data.SP, ret_addr >> 8);    // MSB
        data.set_mem(--data.SP, ret_addr & 0xff);  // LSB
        data.set_PC(addr);                         // jp imm16
    }
    void add_SP(uint8_t byte) {  // adds as signed byte and sets flag
        data.SP = int16_t(byte) + data.SP;
        flags.z = 0;
        flags.n = 0;
        printf("Not yet implemented add data.SP flags\n");
    }

    /* ------------------- condition calls ------------------ */
    clock_cycles ret_if(bool cc) {  // for RET cc
        if (!cc) return 2;
        ret();
        return 5;
    }
    clock_cycles jr_if(bool cc, uint8_t byte) {  // for JR cc
        if (!cc) return 2;
        data.signed_offset_PC(byte);
        return 3;
    }
    clock_cycles jp_if(bool cc, uint16_t addr) {  // for JR cc
        if (!cc) return 3;
        data.set_PC(addr);
        return 4;
    }
    clock_cycles call_if(bool cc, uint16_t addr) {  // for CALL cc
        if (!cc) return 3;
        call(addr);
        return 6;
    }

    /* --------------------- interrupts --------------------- */
    bool interrupt_flag = false;
    bool set_EI = false;
    void check_EI() {  // check for next instruction after EI to enable IME
        if (!set_EI) return;
        interrupt_flag = true;
        set_EI = false;
    }
};
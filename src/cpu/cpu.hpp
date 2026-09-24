#pragma once

#include <stdlib.h>
#include <vector>
#include <iostream>
#include <iomanip>

#include "cpu_data.hpp"
#include "memory.hpp"
#include "opcodes.hpp"
#include "logger.hpp"
#include "clock.hpp"

#define DEBUG_OPCODE
#define DEBUG_INSTR
#ifdef DEBUG_INSTR
#define CODE(msg) LOG_OPCODE_LINE(msg)
#else
#define CODE(...)
#endif
constexpr uint64_t cpu_instr_limit = 1e12;
// constexpr uint cpu_instr_limit = 100;

// F = flags register
struct Flags {
    bool z;  // zero result
    bool n;  // subtract
    bool h;  // half carry
    bool c;  // carry, when addition overflows or subtraction underflows, or when shifting out 1
};

typedef uint8_t clock_cycles;
class CPU {
   public:
    CPU() {}
    void init(MemoryManager* mm, Clock* clock) {
        this->mm = mm;
        this->clock = clock;
    }
    void load_rom(std::string path) { data.load_ROM_from_path(path.c_str()); }
    void load_test_rom(uint8_t idx);
    bool run(bool stop_on_halt = false);
    clock_cycles parse_byte(uint8_t byte);
    void dump_state(std::ofstream& stream, bool show_flags = false);

   private:
    Clock* clock;
    /* ------------------------ data ------------------------ */
    CPUData data;
    MemoryManager* mm;
    uint8_t read_r8(uint8_t idx);
    void set_r8(uint8_t idx, uint8_t val);

    /* ------------------- opcode parsing ------------------- */
    uint8_t opcode;
    OpState op_state = OpState::READY;
    uint16_t imm16;
    uint8_t r16_addr;  // for imm16 stuff
    uint8_t r8_addr;   // for imm8 stuff, also used for jr cond

    uint64_t instr_count = 0;
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
    void set_flags_znhc(bool z, bool n, bool h, bool c);
    void set_flags_nhc(bool n, bool h, bool c);

    /* ---------- 8-bit arithmetics and sets flags ---------- */
    uint8_t calc_add8(uint8_t a, uint8_t b);
    uint8_t calc_adc8(uint8_t a, uint8_t b);
    uint8_t calc_sub8(uint8_t a, uint8_t b);  // use for cp8 as well
    uint8_t calc_sbc8(uint8_t a, uint8_t b);
    uint8_t calc_and8(uint8_t a, uint8_t b);
    uint8_t calc_xor8(uint8_t a, uint8_t b);
    uint8_t calc_or8(uint8_t a, uint8_t b);

    uint16_t read_mem16(uint16_t addr) {
        return mm->read_mem(addr) | (mm->read_mem(addr + 1) << 8);
    }
    void ret();
    void call(uint16_t addr);
    uint16_t add_SP(uint8_t byte);

    /* ------------------- condition calls ------------------ */
    clock_cycles ret_if(bool cc);
    clock_cycles jr_if(bool cc, uint8_t byte);
    clock_cycles jp_if(bool cc, uint16_t addr);
    clock_cycles call_if(bool cc, uint16_t addr);

    /* --------------------- interrupts --------------------- */
    void check_interrupt();
    uint8_t set_EI = 0;  // flag for EI to delay IME enabling by one cycle
    bool IME = false;    // actual IME enabling interrupts
    void check_EI();

    void run_daa();

    bool is_halted = false;
    bool is_stopped = false;
};
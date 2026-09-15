#include <stdlib.h>

#include "registers.hpp"

// F = flags register
struct Flags {
    bool z;  // zero result
    bool n;  // subtract
    bool h;  // half carry
    bool c;  // carry, when addition overflows or subtraction underflows, or when shifting out 1
};

// flags for if immediate bytes need to be read
enum class OpState {
    READY,  // normal opcode processing
    IMM8,
    IMM16L,  // goes to low first
    IMM16H,
    CB,
};

enum OpCodeType {
    NOP = 0,
    HALT = 0b0111'0110,
    LD_IMM16_SP = 0b0000'1000,
    STOP_IMM8 = 0b0001'0000,  // consumes 2 bytes

    JR_IMM8 = 0b0001'1000,
    JR_NZ_IMM8 = 0b001'00'000,
    JR_Z_IMM8 = 0b001'01'000,
    JR_NC_IMM8 = 0b001'10'000,
    JR_C_IMM8 = 0b001'11'000,

    /* ---------------- reg a stuff and flags --------------- */
    RLCA = 0b0000'0111,
    RRCA = 0b0000'1111,
    RLA = 0b0001'0111,
    RRA = 0b0001'1111,
    DAA = 0b0010'0111,  // decimal adjust accumulator
    CPL = 0b0010'1111,  // bitwise not the A register
    SCF = 0b0011'0111,  // set carry flag
    CFF = 0b0011'1111,  // compliment carry flag

    /* ---------------------- reg A ops --------------------- */
    ADD_A_IMM8 = 0b11'000'110,
    ADC_A_IMM8 = 0b11'001'110,
    SUB_A_IMM8 = 0b11'010'110,
    SBC_A_IMM8 = 0b11'011'110,
    AND_A_IMM8 = 0b11'100'110,
    XOR_A_IMM8 = 0b11'101'110,
    OR_A_IMM8 = 0b11'110'110,
    CP_A_IMM8 = 0b11'111'110,

    /* ----------------------- block 3 ---------------------- */
    RET = 0b1100'1001,  // return from subroutine, load SP into PC
    RET_NZ = 0b110'00'000,
    RET_Z = 0b110'01'000,
    RET_NC = 0b110'10'000,
    RET_C = 0b110'11'000,
    RETI = 0b1101'1001,

    JP_IMM16 = 0b1100'0011,
    JP_NZ_IMM16 = 0b110'00'010,
    JP_Z_IMM16 = 0b110'01'010,
    JP_NC_IMM16 = 0b110'10'010,
    JP_C_IMM16 = 0b110'11'010,
    JP_HL = 0b1110'1001,

    CALL_IMM16 = 0b1100'1101,
    CALL_NZ_IMM16 = 0b110'00'100,
    CALL_Z_IMM16 = 0b110'01'100,
    CALL_NC_IMM16 = 0b110'10'100,
    CALL_C_IMM16 = 0b110'11'100,

    /* ------------------ variable opcodes ------------------ */
    LD_r8_IMM8 = UINT8_MAX + 1,
    LD_r16_IMM16,
};

typedef uint8_t clock_cycles;
constexpr size_t SOME_LARGE_NUMBER = 1e10;
class CPU {
   public:
    clock_cycles parse_byte(uint8_t byte);

   private:
    clock_cycles parse_opcode();
    void parse_8bit_arith();
    clock_cycles parse_block0();
    clock_cycles parse_block3();

    // extra data handling
    clock_cycles handle_imm16();
    clock_cycles handle_imm8(uint8_t byte);
    clock_cycles handle_cb(uint8_t byte);

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
    OpState op_state;
    uint16_t imm16;
    uint8_t r16_addr;  // for imm16 stuff
    uint8_t r8_addr;   // for imm8 stuff, also used for jr cond

    uint8_t memory[SOME_LARGE_NUMBER];
    uint16_t read_mem16(uint16_t addr) { return memory[addr] + memory[addr + 1] << 8; }
    void ret() {
        PC = read_mem16(SP);
        SP += 2;
    }
    void call(uint16_t addr) {  // store PC in SP and jump to addr
        SP = PC + 1;
        PC = addr;  // jp imm16
    }

    /* ------------------- condition calls ------------------ */
    clock_cycles ret_if(bool cc) {  // for RET cc
        if (!cc) return 2;
        ret();
        return 5;
    }
    clock_cycles jr_if(bool cc, uint8_t byte) {  // for JR cc
        if (!cc) return 2;
        PC = PC + (int16_t)byte;
        return 3;
    }
    clock_cycles jp_if(bool cc, uint16_t addr) {  // for JR cc
        if (!cc) return 3;
        PC = addr;
        return 4;
    }
    clock_cycles call_if(bool cc, uint16_t addr) {  // for CALL cc
        if (!cc) return 3;
        call(addr);
        return 6;
    }
};
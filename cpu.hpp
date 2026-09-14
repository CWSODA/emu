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
};

enum OpCodeType {
    NOP = 0,
    LD_IMM16_SP = 0b0000'1000,
    STOP_IMM8 = 0b0001'0000,  // consumes 2 bytes
    JR_IMM8 = 0b0001'1000,

    /* ---------------------- reg A ops --------------------- */
    ADD_A_IMM8 = 0b11'000'110,
    ADC_A_IMM8 = 0b11'001'110,
    SUB_A_IMM8 = 0b11'010'110,
    SBC_A_IMM8 = 0b11'011'110,
    AND_A_IMM8 = 0b11'100'110,
    XOR_A_IMM8 = 0b11'101'110,
    OR_A_IMM8 = 0b11'110'110,
    CP_A_IMM8 = 0b11'111'110,

    /* ------------------ variable opcodes ------------------ */
    LD_r8_IMM8,
    LD_r16_IMM16,
    JR_COND_IMM8,
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

    // imm handling
    clock_cycles handle_imm16();
    clock_cycles handle_imm8(uint8_t byte);

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
};
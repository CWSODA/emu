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
    READY,
    IMM8,
    IMM16L,  // goes to low first
    IMM16H,
};

// for opcodes requiring imm16
enum class OpCodeType16 {
    LD_r16_IMM16,
    LD_IMM16_SP,
};
// for opcodes requiring imm8
enum class OpCodeType8 {
    JR_IMM8,
    JR_COND_IMM8,
    LD_r8_IMM8,
};

// non range opcodes
enum OpCodeType {
    NOP = 0,
    HALT = 0b0111'0110,
    LD_IMM16_SP = 0b0000'1000,
    JR_IMM8 = 0b000'11'000,
    /* --------------------- block0 ops --------------------- */
    RLCA = 0b0000'0111,
    RRCA = 0b0000'1111,
    RLA = 0b0001'0111,
    RRA = 0b0001'1111,
    DAA = 0b0010'0111,  // decimal adjust accumulator
    CPL = 0b0010'1111,  // bitwise not register A
    SCF = 0b0011'0111,  // set carry flag
    CCF = 0b0011'1111,  // compliment carry flag
    /* --------------------- block3 ops --------------------- */
    /* ---------------------- reg A ops --------------------- */
    ADC_A_IMM8 = 0b11'000'110,
    ADD_A_IMM8 = 0b11'001'110,
    SUB_A_IMM8 = 0b11'010'110,
    SBC_A_IMM8 = 0b11'011'110,
    AND_A_IMM8 = 0b11'100'110,
    XOR_A_IMM8 = 0b11'101'110,
    OR_A_IMM8 = 0b11'110'110,
    CP_A_IMM8 = 0b11'111'110,

    RET = 0b110'01'001,
    RETI = 0b110'11'001,
    JP_IMM16 = 0b1100'0011,
    JP_HL = 0b1110'1001,
    CALL_IMM16 = 0b1100'1101,

    CB_IMM8 = 0b1100'1011,

    /* ------------------ variable opcodes ------------------ */
    LD_r16_IMM16,
    LD_r8_IMM8,
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
    void parse_block0();
    void parse_block3();

    // imm handling
    void handle_imm16();
    void handle_imm8(uint8_t byte);

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

    uint8_t opcode;
    OpCodeType16 opcode16;
    OpCodeType8 opcode8;
    OpState op_state;
    uint16_t imm16;
    uint8_t r16_addr;  // for imm16 stuff
    uint8_t r8_addr;   // for imm8 stuff, also used for jr cond

    uint8_t memory[SOME_LARGE_NUMBER];
};
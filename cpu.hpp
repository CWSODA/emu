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

enum class OpCodeType {
    /* ------------------------ misc ------------------------ */
    NOP = 0,

    /* ------------------------ imm16 ----------------------- */
    LD_r16_IMM16,
    LD_IMM16_SP = 0b0000'1000,
    /* ------------------------ imm8 ------------------------ */
    JR_IMM8,
    JR_COND_IMM8,
    LD_r8_IMM8,
    /* ---------------------- reg A ops --------------------- */
    ADC_A_IMM8 = 0b11'000'110,
    ADD_A_IMM8 = 0b11'001'110,
    SUB_A_IMM8 = 0b11'010'110,
    SBC_A_IMM8 = 0b11'011'110,
    AND_A_IMM8 = 0b11'100'110,
    XOR_A_IMM8 = 0b11'101'110,
    OR_A_IMM8 = 0b11'110'110,
    CP_A_IMM8 = 0b11'111'110,
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
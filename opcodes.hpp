// flags for if immediate bytes need to be read
enum class OpState {
    READY,  // normal opcode processing
    IMM8,
    IMM16L,  // goes to low first
    IMM16H,
    CB,
};

enum OpCode {
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
    RET = 0b1100'1001,  // return from subroutine, load registers.SP into registers.PC
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

    LDH_C_A = 0b1110'0010,
    LDH_A_C = 0b1111'0010,
    LDH_IMM8_A = 0b1110'0000,
    LDH_A_IMM8 = 0b1111'0000,
    LD_IMM16_A = 0b1110'1010,
    LD_A_IMM16 = 0b1111'1010,

    ADD_SP_IMM8 = 0b1110'1000,  // add signed imm8 to registers.SP
    LD_HL_SP_IMM8 = 0b1111'1000,
    LD_SP_HL = 0b1111'1001,

    DI = 0b1111'0011,  // disable interrupt
    EI = 0b1111'1011,  // enable interrupt, only after next instruction!
    CB_prefix = 0xCB,

    /* ------------------ variable opcodes ------------------ */
    LD_r8_IMM8 = 0b00'000'110,
    LD_r16_IMM16 = 0b00'00'0001,
};

enum CBPrefixOpcode {
    RLC_r8 = 0b00000,
    RRC_r8 = 0b00001,
    RL_r8 = 0b00010,
    RR_r8 = 0b00011,
    SLA_r8 = 0b00100,
    SRA_r8 = 0b00101,
    SWAP_r8 = 0b00110,
    SRL_r8 = 0b00111,
};
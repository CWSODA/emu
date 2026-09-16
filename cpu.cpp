#include <stdlib.h>
#include <stdio.h>

#include "cpu.hpp"

#define DEBUG_OPCODE
#ifdef DEBUG_OPCODE
#define CODE(...) printf(">>> %s\n", __VA_ARGS__)
#else
#define CODE(...)
#endif

clock_cycles CPU::parse_byte(uint8_t byte) {
    switch (op_state) {
        case OpState::READY: {
            opcode = byte;
            return parse_opcode();
        }
        case OpState::IMM8: {
            op_state = OpState::READY;
            return handle_imm8(byte);
        }
        case OpState::CB: {
            op_state = OpState::READY;
            return handle_cb(byte);
        }
        case OpState::IMM16L: {
            imm16 = byte;
            op_state = OpState::IMM16H;
            return 0;
        }
        case OpState::IMM16H: {
            imm16 += byte << 8;
            op_state = OpState::READY;
            return handle_imm16();
        }
    }
    printf("Error parsing byte: %02X\n", byte);
    return 0;
}

clock_cycles CPU::parse_opcode() {
    // match static opcodes first
    switch (opcode) {
        case NOP: {
            CODE("nop");
            return 1;
        }
        case HALT: {
            CODE("halt");
            while (true);
        }
        /* ------------------------ imm8 ------------------------ */
        // reg A arithmetic
        case ADD_A_IMM8:
        case ADC_A_IMM8:
        case SUB_A_IMM8:
        case SBC_A_IMM8:
        case AND_A_IMM8:
        case XOR_A_IMM8:
        case OR_A_IMM8:
        case CP_A_IMM8:
        // jumps
        case JR_IMM8:
        case JR_NZ_IMM8:
        case JR_Z_IMM8:
        case JR_NC_IMM8:
        case JR_C_IMM8:
        // other
        case LDH_IMM8_A:
        case LDH_A_IMM8:
        case ADD_SP_IMM8:
        case LD_HL_SP_IMM8:
        case STOP_IMM8: {
            CODE("IMM8");
            op_state = OpState::IMM8;
            return 0;
        }
        /* ------------------------ imm16 ----------------------- */
        case JP_NZ_IMM16:
        case JP_Z_IMM16:
        case JP_NC_IMM16:
        case JP_C_IMM16:
        case JP_IMM16:
        case CALL_IMM16:
        case LD_IMM16_A:
        case LD_A_IMM16:
        case LD_IMM16_SP: {
            CODE("IMM16");
            op_state = OpState::IMM16L;
            return 0;
        }
        /* ---------------- reg A stuff and flags --------------- */
        case RLCA: {
            CODE("RLCA");
            set_flags_znhc(0, 0, 0, registers.a() & 0b1000'000);
            registers.a() = (registers.a() << 1) | flags.c;
            return 1;
        }
        case RRCA: {
            CODE("RRCA");
            set_flags_znhc(0, 0, 0, registers.a() & 0b1);
            registers.a() = (registers.a() >> 1) | (flags.c << 7);
            return 1;
        }
        case RLA: {
            CODE("RLA");
            bool msb = registers.a() & 0b1000'0000;
            registers.a() = (registers.a() << 1) | flags.c;
            set_flags_znhc(0, 0, 0, msb);
            return 1;
        }
        case RRA: {
            CODE("RRA");
            bool lsb = registers.a() & 1;
            registers.a() = (registers.a() >> 1) | (flags.c << 7);
            set_flags_znhc(0, 0, 0, lsb);
            return 1;
        }
        case DAA: {
            printf("DAA not yet implemented!\n");
            return 1;
        }
        case CPL: {
            CODE("CPL");
            registers.a() = ~registers.a();
            flags.n = 1;
            flags.h = 1;
            return 1;
        }
        case SCF: {
            CODE("SCF");
            set_flags_nhc(0, 0, 1);
            return 1;
        }
        case CFF: {
            CODE("CFF");
            set_flags_nhc(0, 0, !flags.c);
            return 1;
        }
        /* ----------------------- block3 ----------------------- */
        case RET_NZ:
            return ret_if(!flags.z);
        case RET_Z:
            return ret_if(flags.z);
        case RET_NC:
            return ret_if(!flags.c);
        case RET_C:
            return ret_if(flags.c);
        case RET: {
            ret();
            return 4;
        }
        case RETI: {
            ret();
            printf("Interrupts not yet enabled!\n");
            return 4;
        }
        case JP_HL: {
            registers.set_PC(registers.get_r16(2));  // copy HL to registers.PC
            return 1;
        }
        /* ------------------------- ld ------------------------- */
        case LDH_C_A: {
            memory[0xff00 + registers.c()] = registers.a();
            return 2;
        }
        case LDH_A_C: {
            registers.a() = memory[0xff00 + registers.c()];
            return 2;
        }
        case DI: {
            interrupt_flag = false;
            return 1;
        }
        case EI: {
            set_EI = true;
            return 1;
        }
    }
    // match variable opcodes
    switch (opcode >> 6) {
        case 0: {
            puts("block0");
            return parse_block0();
        }
        case 0b01: {  // 8-bit reg to reg load
            if (opcode == 0b0111011) {
                printf("halt called!\n");
            }
            CODE("ld r8, r8");
            uint8_t dest = (opcode >> 3) & 0b111;
            uint8_t src = opcode & 0b111;
            registers.r8(dest) = registers.r8(src);
            return 1;
        }
        case 0b10: {  // 8-bit arithmetic on reg A
            puts("artih");
            parse_8bit_arith();
            return 1;
        }
        case 0b11: {
            puts("block3");
            return parse_block3();
        }
    }
    printf("Error parsing opcode: %02X\n", opcode);
    return 0;
}

void CPU::parse_8bit_arith() {
    uint8_t reg = opcode & 0b111;  // last 3 bits is register
    switch (opcode >> 3) {
        case 0b10'000: {  // add
            CODE("add a, r8");
            registers.a() = calc_add8(registers.a(), registers.r8(reg));
            break;
        }
        case 0b10'001: {  // adc
            CODE("adc a, r8");
            registers.a() = calc_adc8(registers.a(), registers.r8(reg));
            break;
        }
        case 0b10'010: {  // sub
            CODE("sub a, r8");
            registers.a() = calc_sub8(registers.a(), registers.r8(reg));
            break;
        }
        case 0b10'011: {  // sbc
            CODE("sbc a, r8");
            registers.a() = calc_sbc8(registers.a(), registers.r8(reg));
            break;
        }
        case 0b10'100: {  // and
            CODE("and a, r8");
            registers.a() = calc_and8(registers.a(), registers.r8(reg));
            break;
        }
        case 0b10'101: {  // xor
            CODE("xor a, r8");
            registers.a() = calc_xor8(registers.a(), registers.r8(reg));
            break;
        }
        case 0b10'110: {  // or
            CODE("or a, r8");
            registers.a() = calc_or8(registers.a(), registers.r8(reg));
            break;
        }
        case 0b10'111: {  // cp (compare, subtracts without updating register)
            CODE("cp a, r8");
            calc_sub8(registers.a(), registers.r8(reg));
            break;
        }
    }
    printf("Error parsing 8-bit arithmetic opcode: %02X\n", opcode);
}

// used to parse variable opcodes
clock_cycles CPU::parse_block0() {
    uint8_t last_three = (opcode & 0b111);
    uint8_t last_nibble = (opcode & 0b1111);
    if (last_three == 0b100) {  // inc r8
        uint8_t reg = opcode >> 3;
        flags.h = (registers.r8(reg) & 0b1111) == 0b1111;  // overflow into bit 4
        registers.r8(reg) += 1;
        flags.z = (registers.r8(reg) == 0);
        flags.n = false;
    } else if (last_three == 0b101) {  // dec r8
        uint8_t reg = opcode >> 3;
        flags.h = (registers.r8(reg) & 0b1111) == 0;  // borrow from bit 4
        registers.r8(reg) -= 1;
        flags.z = (registers.r8(reg) == 0);
        flags.n = true;
    } else if (last_three == 0b110) {  // ld r8, imm8
        op_state = OpState::IMM8;
        r8_addr = opcode >> 3;
    } else if (last_three == 0b000 && (opcode & 0b0010'0000)) {  // jr cond, imm8
        op_state = OpState::IMM8;
        r8_addr = (opcode >> 3) & 0b11;
    } else if (last_nibble == 0b0001) {  // ld r16, imm16
        op_state = OpState::IMM16L;
        r16_addr = opcode >> 4;
    } else if (last_nibble == 0b0010) {  // ld [r16mem], a
        // r16mem: bc, de, hl+, hl-
        CODE("ld [r16mem], a");
        uint8_t addr = opcode >> 4;
        if (addr == 0)
            memory[registers.get_bc()] = registers.a();
        else if (addr == 1)
            memory[registers.get_de()] = registers.a();
        else {
            uint16_t hl = registers.get_hl();
            memory[hl] = registers.a();
            registers.set_hl(hl + (addr == 2 ? 1 : -1));  // incr / decr
        }
    } else if (last_nibble == 0b1010) {  // ld a, [r16mem]
        // r16mem: bc, de, hl+, hl-
        uint8_t addr = opcode >> 4;
        if (addr == 0)
            registers.a() = memory[registers.get_bc()];
        else if (addr == 1)
            registers.a() = memory[registers.get_de()];
        else {
            uint16_t hl = registers.get_hl();
            registers.a() = memory[hl];
            registers.set_hl(hl + (addr == 2 ? 1 : -1));  // incr / decr
        }

        // 16-bit arithmetic
    } else if (last_nibble == 0b0011) {  // inc r16, 2 cycles
        uint8_t addr = opcode >> 4;
        registers.set_r16(addr, registers.get_r16(addr) + 1);
    } else if (last_nibble == 0b1011) {  // dec r16, 2 cycles
        uint8_t addr = opcode >> 4;
        registers.set_r16(addr, registers.get_r16(addr) - 1);
    } else if (last_nibble == 0b1001) {  // add hl, r16, 2 cycles
        uint16_t hl = registers.get_hl();
        uint16_t val = registers.get_r16(opcode >> 4);
        flags.n = false;
        flags.h = (hl & 0xfff + val & 0xfff) > 0xfff;  // bit 11 overflow
        flags.c = hl > (0xff'ff - val);                // bit 15 overflow
        registers.set_hl(hl + val);
    }
    printf("Error parsing block0 opcode: %02X\n", opcode);
    return 0;
}

clock_cycles CPU::parse_block3() {
    if ((opcode & 0b111) == 0b111) {  // rst tgt3, calls tgt3 * 8
        uint16_t tgt3 = (opcode >> 3) & 0b111;
        call(tgt3 * 8);
        return 4;
    }
    uint8_t r16stk = opcode >> 4 & 0b11;  // cooresponding to bc, de, hl, af
    switch (opcode & 0b1111) {
        case 0b0001: {          // POP r16stk
            if (r16stk <= 2) {  // bc, de, hl registers
                registers.r8(2 * r16stk + 1) = memory[registers.SP++];
                registers.r8(2 * r16stk) = memory[registers.SP++];
                return 3;
            }
            // af register
            flags.z = memory[registers.SP] & (1 << 7);
            flags.n = memory[registers.SP] & (1 << 6);
            flags.h = memory[registers.SP] & (1 << 5);
            flags.c = memory[registers.SP] & (1 << 4);
            registers.SP += 1;
            registers.a() = memory[registers.SP++];
            return 3;
        }
        case 0b0101: {          // PUSH r16stk
            if (r16stk <= 2) {  // bc, de, hl registers
                memory[--registers.SP] = registers.r8(2 * r16stk);
                memory[--registers.SP] = registers.r8(2 * r16stk + 1);
                return 4;
            }
            // af register
            memory[--registers.SP] = registers.a();
            memory[--registers.SP] =
                (flags.z << 7) + (flags.n << 6) + (flags.h << 5) + (flags.c << 4);
            return 4;
        }
    }
    printf("Error handling block 3 opcode: %02X\n", opcode);
    return 0;
}

clock_cycles CPU::handle_imm16() {
    switch (opcode) {
        /* ------------------------ jump ------------------------ */
        case JP_IMM16: {
            CODE("jp imm16");
            registers.set_PC(imm16);
            return 4;
        }
        case JP_NZ_IMM16:
            return jp_if(!flags.z, imm16);
        case JP_Z_IMM16:
            return jp_if(flags.z, imm16);
        case JP_NC_IMM16:
            return jp_if(!flags.c, imm16);
        case JP_C_IMM16:
            return jp_if(flags.c, imm16);

        /* ------------------------ call ------------------------ */
        case CALL_IMM16: {
            call(imm16);
            return 6;
        }
        case CALL_NZ_IMM16:
            return call_if(!flags.z, imm16);
        case CALL_Z_IMM16:
            return call_if(flags.z, imm16);
        case CALL_NC_IMM16:
            return call_if(!flags.c, imm16);
        case CALL_C_IMM16:
            return call_if(flags.c, imm16);

        /* ------------------------ load ------------------------ */
        case LD_IMM16_SP: {
            memory[imm16] = registers.SP & 0xff;
            memory[imm16 + 1] = registers.SP >> 8;
            return 5;
        }
        case LD_r16_IMM16: {
            registers.set_r16(r16_addr, imm16);
            return 3;
        }
        case LD_IMM16_A: {
            memory[imm16] = registers.a();
            return 4;
        }
        case LD_A_IMM16: {
            registers.a() = memory[imm16];
            return 4;
        }
    }
    printf("Error handling imm16 opcode: %02X\n", opcode);
    return 0;
}

clock_cycles CPU::handle_imm8(uint8_t byte) {
    switch (opcode) {
        /* ------------------------- jr ------------------------- */
        case JR_IMM8: {
            registers.signed_offset_PC(byte);
            return 3;
        }
        case JR_NZ_IMM8:
            return jr_if(!flags.z, byte);
        case JR_Z_IMM8:
            return jr_if(flags.z, byte);
        case JR_NC_IMM8:
            return jr_if(!flags.c, byte);
        case JR_C_IMM8:
            return jr_if(flags.c, byte);
        case LD_r8_IMM8: {  // 2 cycles
            registers.r8(r8_addr) = byte;
            return 2;
        }
        /* ------------------- reg a imm8 ops ------------------- */
        case ADD_A_IMM8: {
            registers.a() = calc_add8(registers.a(), byte);
            return 2;
        }
        case ADC_A_IMM8: {
            registers.a() = calc_adc8(registers.a(), byte);
            return 2;
        }
        case SUB_A_IMM8: {
            registers.a() = calc_sub8(registers.a(), byte);
            return 2;
        }
        case SBC_A_IMM8: {
            registers.a() = calc_sbc8(registers.a(), byte);
            return 2;
        }
        case AND_A_IMM8: {
            registers.a() = calc_and8(registers.a(), byte);
            return 2;
        }
        case XOR_A_IMM8: {
            registers.a() = calc_xor8(registers.a(), byte);
            return 2;
        }
        case OR_A_IMM8: {
            registers.a() = calc_or8(registers.a(), byte);
            return 2;
        }
        case CP_A_IMM8: {
            calc_sub8(registers.a(), byte);
            return 2;
        }
        /* ----------------------- block3 ----------------------- */
        case LDH_IMM8_A: {
            memory[0xff00 + byte] = registers.a();
            return 3;
        }
        case LDH_A_IMM8: {
            registers.a() = memory[0xff00 + byte];
            return 3;
        }
        case ADD_SP_IMM8: {
            add_SP(byte);
            return 4;
        }
        case LD_HL_SP_IMM8: {
            add_SP(byte);
            registers.set_hl(registers.SP);
            return 3;
        }
    }
    printf("Error handling imm8 opcode: %02X\n", opcode);
    return 0;
}

clock_cycles CPU::handle_cb(uint8_t byte) {
    switch (byte) {}
    puts("$CB prefix opcodes not yet implemented!\n");
    return 0;
}
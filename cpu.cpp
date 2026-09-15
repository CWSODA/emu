#include <stdlib.h>
#include <stdio.h>

#include "cpu.hpp"

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
            break;
        }
        case OpState::IMM16H: {
            imm16 += byte << 8;
            op_state = OpState::READY;
            return handle_imm16();
        }
    }
}

clock_cycles CPU::parse_opcode() {
    // match static opcodes first
    switch (opcode) {
        case NOP: {
            return 1;
        }
        case HALT: {
            printf("Halting program!\n");
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
        case STOP_IMM8: {
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
        case LD_IMM16_SP: {
            op_state = OpState::IMM16L;
            return 0;
        }
        /* ---------------- reg A stuff and flags --------------- */
        case RLCA: {
            set_flags_znhc(0, 0, 0, reg_a & 0b1000'000);
            reg_a = (reg_a << 1) | flags.c;
            return 1;
        }
        case RRCA: {
            set_flags_znhc(0, 0, 0, reg_a & 0b1);
            reg_a = (reg_a >> 1) | (flags.c << 7);
            return 1;
        }
        case RLA: {
            bool msb = reg_a & 0b1000'0000;
            reg_a = (reg_a << 1) | flags.c;
            set_flags_znhc(0, 0, 0, msb);
            return 1;
        }
        case RRA: {
            bool lsb = reg_a & 1;
            reg_a = (reg_a >> 1) | (flags.c << 7);
            set_flags_znhc(0, 0, 0, lsb);
            return 1;
        }
        case DAA: {
            printf("DAA not yet implemented!\n");
            return 1;
        }
        case CPL: {
            reg_a = ~reg_a;
            flags.n = 1;
            flags.h = 1;
            return 1;
        }
        case SCF: {
            set_flags_nhc(0, 0, 1);
            return 1;
        }
        case CFF: {
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
            PC = get_r16(2);  // copy HL to PC
            return 1;
        }
    }
    // match variable opcodes
    switch (opcode >> 6) {
        case 0: {
            parse_block0();
            break;
        }
        case 0b01: {  // 8-bit reg to reg load
            if (opcode == 0b0111011) {
                printf("halt called!\n");
            }
            uint8_t dest = (opcode >> 3) & 0b111;
            uint8_t src = opcode & 0b111;
            r8(dest) = r8(src);
            return 1;
        }
        case 0b10: {  // 8-bit arithmetic on reg A
            parse_8bit_arith();
            return 1;
        }
        case 0b11: {
            parse_block3();
            break;
        }
    }
}

void CPU::parse_8bit_arith() {
    uint8_t reg = opcode & 0b111;  // last 3 bits is register
    switch (opcode >> 3) {
        case 0b10'000: {  // add
            reg_a = calc_add8(reg_a, r8(reg));
            break;
        }
        case 0b10'001: {  // adc
            reg_a = calc_adc8(reg_a, r8(reg));
            break;
        }
        case 0b10'010: {  // sub
            reg_a = calc_sub8(reg_a, r8(reg));
            break;
        }
        case 0b10'011: {  // sbc
            reg_a = calc_sbc8(reg_a, r8(reg));
            break;
        }
        case 0b10'100: {  // and
            reg_a = calc_and8(reg_a, r8(reg));
            break;
        }
        case 0b10'101: {  // xor
            reg_a = calc_xor8(reg_a, r8(reg));
            break;
        }
        case 0b10'110: {  // or
            reg_a = calc_or8(reg_a, r8(reg));
            break;
        }
        case 0b10'111: {  // cp (compare, subtracts without updating register)
            calc_sub8(reg_a, r8(reg));
            break;
        }
    }
}

// used to parse variable opcodes
clock_cycles CPU::parse_block0() {
    uint8_t last_three = (opcode & 0b111);
    uint8_t last_nibble = (opcode & 0b1111);
    switch (opcode) {
        case 0b0001'0000: {            // stop
            op_state = OpState::IMM8;  // second byte usually discarded
            break;
        }
        case 0b0000'1000: {  // ld [imm16] sp
            op_state = OpState::IMM16L;
            break;
        }
        case 0b000'11'000: {  // jr imm8
            op_state = OpState::IMM8;
            break;
        }
        // rest have variable stuff
        default: {
        }
    }
    if (last_three == 0b100) {  // inc r8
        uint8_t reg = opcode >> 3;
        flags.h = (r8(reg) & 0b1111) == 0b1111;  // overflow into bit 4
        r8(reg) += 1;
        flags.z = (r8(reg) == 0);
        flags.n = false;
    } else if (last_three == 0b101) {  // dec r8
        uint8_t reg = opcode >> 3;
        flags.h = (r8(reg) & 0b1111) == 0;  // borrow from bit 4
        r8(reg) -= 1;
        flags.z = (r8(reg) == 0);
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
        uint8_t addr = opcode >> 4;
        if (addr == 0)
            memory[get_bc()] = reg_a;
        else if (addr == 1)
            memory[get_de()] = reg_a;
        else {
            uint16_t hl = get_hl();
            memory[hl] = reg_a;
            set_hl(hl + (addr == 2 ? 1 : -1));  // incr / decr
        }
    } else if (last_nibble == 0b1010) {  // ld a, [r16mem]
        // r16mem: bc, de, hl+, hl-
        uint8_t addr = opcode >> 4;
        if (addr == 0)
            reg_a = memory[get_bc()];
        else if (addr == 1)
            reg_a = memory[get_de()];
        else {
            uint16_t hl = get_hl();
            reg_a = memory[hl];
            set_hl(hl + (addr == 2 ? 1 : -1));  // incr / decr
        }

        // 16-bit arithmetic
    } else if (last_nibble == 0b0011) {  // inc r16, 2 cycles
        uint8_t addr = opcode >> 4;
        set_r16(addr, get_r16(addr) + 1);
    } else if (last_nibble == 0b1011) {  // dec r16, 2 cycles
        uint8_t addr = opcode >> 4;
        set_r16(addr, get_r16(addr) - 1);
    } else if (last_nibble == 0b1001) {  // add hl, r16, 2 cycles
        uint16_t hl = get_hl();
        uint16_t val = get_r16(opcode >> 4);
        flags.n = false;
        flags.h = (hl & 0xfff + val & 0xfff) > 0xfff;  // bit 11 overflow
        flags.c = hl > (0xff'ff - val);                // bit 15 overflow
        set_hl(hl + val);
    }
}

clock_cycles CPU::parse_block3() {
    switch (opcode) {
        case ADD_A_IMM8:
        case ADC_A_IMM8:
        case SUB_A_IMM8:
        case SBC_A_IMM8:
        case AND_A_IMM8:
        case XOR_A_IMM8:
        case OR_A_IMM8:
        case CP_A_IMM8: {
            op_state = OpState::IMM8;
            return 0;
        }
    }
}

clock_cycles CPU::handle_imm16() {
    switch (opcode) {
        case LD_IMM16_SP: {
            memory[imm16] = SP & 0xff;
            memory[imm16 + 1] = SP >> 8;
            return 5;
        }
        case LD_r16_IMM16: {
            set_r16(r16_addr, imm16);
            return 3;
        }

        case JP_IMM16: {
            PC = imm16;
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
    }
}

clock_cycles CPU::handle_imm8(uint8_t byte) {
    switch (opcode) {
        /* ------------------------- jr ------------------------- */
        case JR_IMM8: {
            PC = PC + (int16_t)byte;
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
            r8(r8_addr) = byte;
            return 2;
        }
        /* ------------------- reg a imm8 ops ------------------- */
        case ADD_A_IMM8: {
            reg_a = calc_add8(reg_a, byte);
            return 2;
        }
        case ADC_A_IMM8: {
            reg_a = calc_adc8(reg_a, byte);
            return 2;
        }
        case SUB_A_IMM8: {
            reg_a = calc_sub8(reg_a, byte);
            return 2;
        }
        case SBC_A_IMM8: {
            reg_a = calc_sbc8(reg_a, byte);
            return 2;
        }
        case AND_A_IMM8: {
            reg_a = calc_and8(reg_a, byte);
            return 2;
        }
        case XOR_A_IMM8: {
            reg_a = calc_xor8(reg_a, byte);
            return 2;
        }
        case OR_A_IMM8: {
            reg_a = calc_or8(reg_a, byte);
            return 2;
        }
        case CP_A_IMM8: {
            calc_sub8(reg_a, byte);
            return 2;
        }
    }
}

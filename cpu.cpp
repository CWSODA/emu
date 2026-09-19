#include <stdlib.h>
#include <stdio.h>

#include "cpu.hpp"

#define DEBUG_OPCODE
bool CPU::run() {
    auto byte = data.read_mem(data.get_PC());

#ifdef DEBUG_OPCODE
    std::cout << "PC=0x" << std::hex << std::setw(4) << std::setfill('0') << data.get_PC()
              << " || opcode=0x" << std::setw(2) << static_cast<int>(byte) << " | "
              << cvt_binary(byte) << " ||| " << instr_count << '\n';
#endif
    data.inc_PC();
    auto cycles = parse_byte(byte);

    static uint count = 0;
    if (cycles == 255) count++;
    return (count < 1) && (instr_count <= 100'000);  // returns false for halt
}

clock_cycles CPU::parse_byte(uint8_t byte) {
    switch (op_state) {
        case OpState::READY: {
            opcode = byte;
            auto cycles = parse_opcode();
            if (op_state == OpState::READY) instr_count++;  // 1 byte instr
            return cycles;
        }
        case OpState::IMM8: {
            op_state = OpState::READY;
            instr_count++;
            return handle_imm8(byte);
        }
        case OpState::CB: {
            op_state = OpState::READY;
            instr_count++;
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
            instr_count++;
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
            return 255;
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
        case CALL_NZ_IMM16:
        case CALL_Z_IMM16:
        case CALL_NC_IMM16:
        case CALL_C_IMM16:
        case LD_IMM16_A:
        case LD_A_IMM16:
        case LD_IMM16_SP: {
            op_state = OpState::IMM16L;
            return 0;
        }
        /* ---------------- reg A stuff and flags --------------- */
        case RLCA: {
            CODE("RLCA");
            set_flags_znhc(0, 0, 0, data.a() & 0b1000'000);
            data.a() = (data.a() << 1) | flags.c;
            return 1;
        }
        case RRCA: {
            CODE("RRCA");
            set_flags_znhc(0, 0, 0, data.a() & 0b1);
            data.a() = (data.a() >> 1) | (flags.c << 7);
            return 1;
        }
        case RLA: {
            CODE("RLA");
            bool msb = data.a() & 0b1000'0000;
            data.a() = (data.a() << 1) | flags.c;
            set_flags_znhc(0, 0, 0, msb);
            return 1;
        }
        case RRA: {
            CODE("RRA");
            bool lsb = data.a() & 1;
            data.a() = (data.a() >> 1) | (flags.c << 7);
            set_flags_znhc(0, 0, 0, lsb);
            return 1;
        }
        case DAA: {
            printf("DAA not yet implemented!\n");
            return 1;
        }
        case CPL: {
            CODE("CPL");
            data.a() = ~data.a();
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
            CODE("ret nz");
            return ret_if(!flags.z);
        case RET_Z:
            CODE("ret z");
            return ret_if(flags.z);
        case RET_NC:
            CODE("ret nc");
            return ret_if(!flags.c);
        case RET_C:
            CODE("ret c");
            return ret_if(flags.c);
        case RET: {
            CODE("ret");
            ret();
            return 4;
        }
        case RETI: {
            CODE("reti");
            ret();
            printf("Interrupts not yet enabled!\n");
            return 4;
        }
        case JP_HL: {
            CODE("jp hl");
            data.set_PC(data.get_r16(2));  // copy HL to data.PC
            return 1;
        }
        /* ------------------------- ld ------------------------- */
        case LDH_C_A: {
            CODE("ldh c, a");
            data.set_mem(0xff00 + data.c(), data.a());
            return 2;
        }
        case LDH_A_C: {
            CODE("ldh a, c");
            data.a() = data.read_mem(0xff00 + data.c());
            return 2;
        }
        case DI: {
            CODE("di");
            interrupt_flag = false;
            return 1;
        }
        case EI: {
            CODE("ei");
            set_EI = true;
            return 1;
        }
    }
    // match variable opcodes
    switch (opcode >> 6) {
        case 0: {
            return parse_block0();
        }
        case 0b01: {  // 8-bit reg to reg load
            if (opcode == 0b0111011) {
                printf("halt called!\n");
            }
            CODE("ld r8, r8");
            uint8_t dest = (opcode >> 3) & 0b111;
            uint8_t src = opcode & 0b111;
            data.r8(dest) = data.r8(src);
            return 1;
        }
        case 0b10: {  // 8-bit arithmetic on reg A
            parse_8bit_arith();
            return 1;
        }
        case 0b11: {
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
            data.a() = calc_add8(data.a(), data.r8(reg));
            break;
        }
        case 0b10'001: {  // adc
            CODE("adc a, r8");
            data.a() = calc_adc8(data.a(), data.r8(reg));
            break;
        }
        case 0b10'010: {  // sub
            CODE("sub a, r8");
            data.a() = calc_sub8(data.a(), data.r8(reg));
            break;
        }
        case 0b10'011: {  // sbc
            CODE("sbc a, r8");
            data.a() = calc_sbc8(data.a(), data.r8(reg));
            break;
        }
        case 0b10'100: {  // and
            CODE("and a, r8");
            data.a() = calc_and8(data.a(), data.r8(reg));
            break;
        }
        case 0b10'101: {  // xor
            CODE("xor a, r8");
            data.a() = calc_xor8(data.a(), data.r8(reg));
            break;
        }
        case 0b10'110: {  // or
            CODE("or a, r8");
            data.a() = calc_or8(data.a(), data.r8(reg));
            break;
        }
        case 0b10'111: {  // cp (compare, subtracts without updating register)
            CODE("cp a, r8");
            calc_sub8(data.a(), data.r8(reg));
            break;
        }
        default:
            printf("Error parsing 8-bit arithmetic opcode: %02X\n", opcode);
    }
}

// used to parse variable opcodes
clock_cycles CPU::parse_block0() {
    uint8_t last_three = (opcode & 0b111);
    uint8_t last_nibble = (opcode & 0b1111);
    /* ---------------- opcodes with IMM8/16 ---------------- */
    if (last_three == 0b110) {  // ld r8, imm8
        op_state = OpState::IMM8;
        r8_addr = opcode >> 3;
        opcode = LD_r8_IMM8;
        return 0;
    }
    if (last_nibble == 0b0001) {  // ld r16, imm16
        op_state = OpState::IMM16L;
        r16_addr = opcode >> 4;
        opcode = LD_r16_IMM16;
        return 0;
    }
    /* -------------- regular one pass opcodes -------------- */
    if (last_three == 0b100) {  // inc r8
        CODE("inc r8");
        uint8_t reg = opcode >> 3;
        flags.h = (data.r8(reg) & 0b1111) == 0b1111;  // overflow into bit 4
        data.r8(reg) += 1;
        flags.z = (data.r8(reg) == 0);
        flags.n = false;
        printf("Reg inc (%u): %02X\n", reg, data.r8(reg));
        return 1;
    }
    if (last_three == 0b101) {  // dec r8
        CODE("dec r8");
        uint8_t reg = opcode >> 3;
        flags.h = (data.r8(reg) & 0b1111) == 0;  // borrow from bit 4
        data.r8(reg) -= 1;
        flags.z = (data.r8(reg) == 0);
        flags.n = true;
        printf("Reg dec (%u): %02X\n", reg, data.r8(reg));
        return 1;
    }
    if (last_nibble == 0b0010) {  // ld [r16mem], a
        // r16mem: bc, de, hl+, hl-
        CODE("ld [r16mem], a");
        uint8_t addr = opcode >> 4;
        if (addr == 0)
            data.set_mem(data.get_bc(), data.a());
        else if (addr == 1)
            data.set_mem(data.get_de(), data.a());
        else {
            uint16_t hl = data.get_hl();
            data.set_mem(hl, data.a());
            data.set_hl(hl + (addr == 2 ? 1 : -1));  // incr / decr
        }
        return 2;
    }
    if (last_nibble == 0b1010) {  // ld a, [r16mem]
        // r16mem: bc, de, hl+, hl-
        CODE("ld a, [r16mem]");
        uint8_t addr = opcode >> 4;
        if (addr == 0)
            data.a() = data.read_mem(data.get_bc());
        else if (addr == 1)
            data.a() = data.read_mem(data.get_de());
        else {
            uint16_t hl = data.get_hl();
            data.a() = data.read_mem(hl);
            data.set_hl(hl + (addr == 2 ? 1 : -1));  // incr / decr
        }
        return 2;
    }
    // 16-bit arithmetic
    if (last_nibble == 0b0011) {  // inc r16
        CODE("inc r16");
        uint8_t addr = opcode >> 4;
        data.set_r16(addr, data.get_r16(addr) + 1);
        return 2;
    }
    if (last_nibble == 0b1011) {  // dec r16, 2 cycles
        CODE("dec r16");
        uint8_t addr = opcode >> 4;
        data.set_r16(addr, data.get_r16(addr) - 1);
        return 2;
    }
    if (last_nibble == 0b1001) {  // add hl, r16, 2 cycles
        CODE("add hl, r16");
        uint16_t hl = data.get_hl();
        uint16_t val = data.get_r16(opcode >> 4);
        flags.n = false;
        flags.h = (hl & 0xfff + val & 0xfff) > 0xfff;  // bit 11 overflow
        flags.c = hl > (0xff'ff - val);                // bit 15 overflow
        data.set_hl(hl + val);
        return 2;
    }
    printf("Error parsing block0 opcode: 0x%02X\n", opcode);
    return 0;
}

clock_cycles CPU::parse_block3() {
    if ((opcode & 0b111) == 0b111) {  // rst tgt3, calls tgt3 * 8
        CODE("rst tgt3");
        uint16_t tgt3 = (opcode >> 3) & 0b111;
        call(tgt3 * 8);
        return 4;
    }
    uint8_t r16stk = opcode >> 4 & 0b11;  // cooresponding to bc, de, hl, af
    switch (opcode & 0b1111) {
        case 0b0001: {  // POP r16stk
            CODE("pop r16stk");
            if (r16stk <= 2) {  // bc, de, hl data
                data.r8(2 * r16stk + 1) = data.read_mem(data.SP++);
                data.r8(2 * r16stk) = data.read_mem(data.SP++);
                return 3;
            }
            // af register
            flags.z = data.read_mem(data.SP) & (1 << 7);
            flags.n = data.read_mem(data.SP) & (1 << 6);
            flags.h = data.read_mem(data.SP) & (1 << 5);
            flags.c = data.read_mem(data.SP) & (1 << 4);
            data.SP += 1;
            data.a() = data.read_mem(data.SP++);
            return 3;
        }
        case 0b0101: {  // PUSH r16stk
            CODE("push r16stk");
            if (r16stk <= 2) {  // bc, de, hl data
                data.set_mem(--data.SP, data.r8(2 * r16stk));
                data.set_mem(--data.SP, data.r8(2 * r16stk + 1));
                return 4;
            }
            // af register
            uint8_t push_flags = (flags.z << 7) | (flags.n << 6) | (flags.h << 5) | (flags.c << 4);
            data.set_mem(--data.SP, data.a());
            data.set_mem(--data.SP, push_flags);
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
            data.set_PC(imm16);
            return 4;
        }
        case JP_NZ_IMM16:
            CODE("jp nz, imm16");
            return jp_if(!flags.z, imm16);
        case JP_Z_IMM16:
            CODE("jp z, imm16");
            return jp_if(flags.z, imm16);
        case JP_NC_IMM16:
            CODE("jp nc, imm16");
            return jp_if(!flags.c, imm16);
        case JP_C_IMM16:
            CODE("jp c, imm16");
            return jp_if(flags.c, imm16);

        /* ------------------------ call ------------------------ */
        case CALL_IMM16: {
            CODE("call imm16");
            call(imm16);
            return 6;
        }
        case CALL_NZ_IMM16:
            CODE("call nz, imm16");
            return call_if(!flags.z, imm16);
        case CALL_Z_IMM16:
            CODE("call z, imm16");
            return call_if(flags.z, imm16);
        case CALL_NC_IMM16:
            CODE("call nc, imm16");
            return call_if(!flags.c, imm16);
        case CALL_C_IMM16:
            CODE("call c, imm16");
            return call_if(flags.c, imm16);

        /* ------------------------ load ------------------------ */
        case LD_IMM16_SP: {
            CODE("ld [imm16], sp");
            data.set_mem(imm16, data.SP & 0xff);
            data.set_mem(imm16 + 1, data.SP >> 8);
            return 5;
        }
        case LD_r16_IMM16: {
            CODE("ld r16, imm16");
            data.set_r16(r16_addr, imm16);
            return 3;
        }
        case LD_IMM16_A: {
            CODE("ld [imm16], a");
            data.set_mem(imm16, data.a());
            return 4;
        }
        case LD_A_IMM16: {
            CODE("ld a, [imm16]");
            data.a() = data.read_mem(imm16);
            return 4;
        }
    }
    printf("Error handling imm16 opcode: 0x%02X\n", opcode);
    return 0;
}

clock_cycles CPU::handle_imm8(uint8_t byte) {
    switch (opcode) {
        case STOP_IMM8: {
            CODE("stop");
            return 0;
        }
        /* ------------------------- jr ------------------------- */
        case JR_IMM8: {
            CODE("jr imm8");
            data.signed_offset_PC(byte);
            return 3;
        }
        case JR_NZ_IMM8:
            CODE("jr nz, imm8");
            printf("Flag Z: %u\n", flags.z);
            return jr_if(!flags.z, byte);
        case JR_Z_IMM8:
            CODE("jr z, imm8");
            return jr_if(flags.z, byte);
        case JR_NC_IMM8:
            CODE("jr nc, imm8");
            return jr_if(!flags.c, byte);
        case JR_C_IMM8:
            CODE("jr c, imm8");
            return jr_if(flags.c, byte);
        case LD_r8_IMM8: {  // 2 cycles
            CODE("ld r8, imm8");
            data.r8(r8_addr) = byte;
            return 2;
        }
        /* ------------------- reg a imm8 ops ------------------- */
        case ADD_A_IMM8: {
            CODE("add a, imm8");
            data.a() = calc_add8(data.a(), byte);
            return 2;
        }
        case ADC_A_IMM8: {
            CODE("adc a, imm8");
            data.a() = calc_adc8(data.a(), byte);
            return 2;
        }
        case SUB_A_IMM8: {
            CODE("sub a, imm8");
            data.a() = calc_sub8(data.a(), byte);
            return 2;
        }
        case SBC_A_IMM8: {
            CODE("sbc a, imm8");
            data.a() = calc_sbc8(data.a(), byte);
            return 2;
        }
        case AND_A_IMM8: {
            CODE("and a, imm8");
            data.a() = calc_and8(data.a(), byte);
            return 2;
        }
        case XOR_A_IMM8: {
            CODE("xor a, imm8");
            data.a() = calc_xor8(data.a(), byte);
            return 2;
        }
        case OR_A_IMM8: {
            CODE("or a, imm8");
            data.a() = calc_or8(data.a(), byte);
            return 2;
        }
        case CP_A_IMM8: {
            CODE("cp a, imm8");
            calc_sub8(data.a(), byte);
            return 2;
        }
        /* ----------------------- block3 ----------------------- */
        case LDH_IMM8_A: {
            CODE("ldh imm8, a");
            data.set_mem(0xff00 + byte, data.a());
            return 3;
        }
        case LDH_A_IMM8: {
            CODE("ldh a, imm8");
            data.a() = data.read_mem(0xff00 + byte);
            return 3;
        }
        case ADD_SP_IMM8: {
            CODE("add sp, imm8");
            add_SP(byte);
            return 4;
        }
        case LD_HL_SP_IMM8: {
            CODE("ld hl, sp + imm8");
            add_SP(byte);
            data.set_hl(data.SP);
            return 3;
        }
    }
    printf("Error handling imm8 opcode: 0x%02X\n", opcode);
    return 0;
}

clock_cycles CPU::handle_cb(uint8_t byte) {
    switch (byte) {}
    puts("$CB prefix opcodes not yet implemented!\n");
    return 0;
}
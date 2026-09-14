#include <stdlib.h>
#include <stdio.h>

#include "cpu.hpp"

clock_cycles CPU::parse_byte(uint8_t byte) {
    switch (op_state) {
        case OpState::READY: {
            opcode = byte;
            parse_opcode();
            break;
        }
        case OpState::IMM8: {
            handle_imm8(byte);
            op_state = OpState::READY;
            break;
        }
        case OpState::IMM16L: {
            imm16 = byte;
            op_state = OpState::IMM16H;
            break;
        }
        case OpState::IMM16H: {
            imm16 += byte << 8;
            handle_imm16();
            op_state = OpState::READY;
            break;
        }
    }
}

clock_cycles CPU::parse_opcode() {
    // compare first two bits
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
    bool old_c = flags.c;
    switch (opcode >> 3) {
        case 0b10'000: {  // add
            flags.c = reg_a > (0xff - r8(reg));
            flags.h = (reg_a & 0x0f + r8(reg) & 0x0f) > 0x0f;
            reg_a += r8(reg);  // change reg
            flags.z = (reg_a == 0);
            flags.n = false;
            break;
        }
        case 0b10'001: {  // adc
            flags.c = (static_cast<uint16_t>(reg_a) + r8(reg) + old_c) > 0xff;
            flags.h = (reg_a & 0x0f + r8(reg) & 0x0f + old_c) > 0x0f;
            reg_a += r8(reg) + old_c;  // change reg
            flags.z = (reg_a == 0);
            flags.n = false;
            break;
        }
        case 0b10'010: {                                  // sub
            flags.c = reg_a < r8(reg);                    // underflow
            flags.h = (reg_a & 0x0f) < (r8(reg) & 0x0f);  // borrow from bit4
            reg_a -= r8(reg);                             // change reg
            flags.z = (reg_a == 0);
            flags.n = true;
            break;
        }
        case 0b10'011: {                                             // subc
            flags.c = reg_a < (static_cast<uint16_t>(r8(reg)) + 1);  // underflow
            flags.h = (reg_a & 0x0f) < (r8(reg) & 0x0f + 1);         // borrow from bit4
            reg_a -= r8(reg) + old_c;                                // change reg
            flags.z = (reg_a == 0);
            flags.n = true;
            break;
        }
        case 0b10'100: {       // and
            reg_a &= r8(reg);  // change reg
            flags.z = (reg_a == 0);
            flags.h = true;
            flags.c = false;
            flags.n = false;
            break;
            break;
        }
        case 0b10'101: {       // xor
            reg_a ^= r8(reg);  // change reg
            flags.z = (reg_a == 0);
            flags.c = false;
            flags.h = false;
            flags.n = false;
            break;
        }
        case 0b10'110: {       // or
            reg_a |= r8(reg);  // change reg
            flags.z = (reg_a == 0);
            flags.c = false;
            flags.h = false;
            flags.n = false;
            break;
        }
        case 0b10'111: {                                  // cp (compare)
            flags.c = reg_a < r8(reg);                    // underflow
            flags.h = (reg_a & 0x0f) < (r8(reg) & 0x0f);  // borrow from bit4
            uint8_t result = reg_a - r8(reg);
            flags.z = !(bool)result;
            flags.n = true;
            break;
        }
    }
}

// returns cycles used
void CPU::parse_block0() {
    switch (opcode) {
        case 0:  // nop
            return;
        case 0b0001'0000: {            // stop
            op_state = OpState::IMM8;  // second byte usually discarded
            break;
        }
        case 0b0000'1000: {  // ld [imm16] sp
            op_state = OpState::IMM16L;
            opcode16 = OpCodeType16::LD_IMM16_SP;
            break;
        }
        case 0b000'11'000: {  // jr imm8
            op_state = OpState::IMM8;
            break;
        }
        /* ---------------- reg a stuff and flags --------------- */
        case 0b0000'0111: {  // rlca
            set_flags_znhc(0, 0, 0, reg_a & 0b1000'000);
            reg_a <<= 1;
            break;
        }
        case 0b0000'1111: {  // rrca
            set_flags_znhc(0, 0, 0, reg_a & 0b1);
            reg_a >>= 1;
            break;
        }
        case 0b0001'0111: {  // rla
            bool msb = reg_a & 0b1000'0000;
            reg_a <<= 1;
            reg_a += flags.c;
            set_flags_znhc(0, 0, 0, msb);
            break;
        }
        case 0b0001'1111: {  // rra
            bool lsb = reg_a & 1;
            reg_a >>= 1;
            reg_a += flags.c << 8;
            set_flags_znhc(0, 0, 0, lsb);
            break;
        }
        case 0b0010'0111: {  // daa (decimal adjust accumulator)
            printf("DAA not yet implemented!\n");
            break;
        }
        case 0b0010'1111: {  // cpl (basically NOT)
            reg_a = ~reg_a;
            flags.n = 1;
            flags.h = 1;
            break;
        }
        case 0b0011'0111: {  // scf (set carry flag)
            set_flags_nhc(0, 0, 1);
            break;
        }
        case 0b0011'1111: {  // ccf (compliment carry flag)
            set_flags_nhc(0, 0, !flags.c);
            break;
        }
        // rest have variable stuff
        default: {
        }
    }
    //
    uint8_t last_three = (opcode & 0b111);
    uint8_t last_nibble = (opcode & 0b1111);
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
        opcode8 = OpCodeType8::LD_r8_IMM8;
        r8_addr = opcode >> 3;
    } else if (last_three == 0b000 && (opcode & 0b0010'0000)) {  // jr cond, imm8
        op_state = OpState::IMM8;
        opcode8 = OpCodeType8::JR_COND_IMM8;
        r8_addr = (opcode >> 3) & 0b11;
    } else if (last_nibble == 0b0001) {  // ld r16, imm16
        opcode16 = OpCodeType16::LD_r16_IMM16;
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

void CPU::parse_block0() {
    switch (opcode) {
        case 0b1100'0110: {  // add a imm8
            break;
        }
    }
}

void CPU::handle_imm16() {
    switch (opcode16) {
        case OpCodeType16::LD_IMM16_SP: {  // 5 cycles
            memory[imm16] = SP & 0xff;
            memory[imm16 + 1] = SP >> 8;
            break;
        }
        case OpCodeType16::LD_r16_IMM16: {  // 3 cycles
            set_r16(r16_addr, imm16);
            break;
        }
    }
}

void CPU::handle_imm8(uint8_t byte) {
    switch (opcode8) {
        case OpCodeType8::JR_IMM8: {  // jr imm8
            PC = PC + (int16_t)byte;
            break;
        }
        case OpCodeType8::LD_r8_IMM8: {  // jr cond imm8
            // conditions nz, z, nc, c
            if ((r8_addr == 0 && !flags.z) | (r8_addr == 1 && flags.z) |
                (r8_addr == 2 && !flags.c) | (r8_addr == 3 && flags.c))
                PC = PC + (int16_t)byte;
            break;
        }
        case OpCodeType8::JR_COND_IMM8: {  // 2 cycles
            r8(r8_addr) = byte;
            break;
        }
    }
}
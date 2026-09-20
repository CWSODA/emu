#include "cpu.hpp"

/* --------------------- arithmetic --------------------- */
uint8_t CPU::calc_add8(uint8_t a, uint8_t b) {
    flags.n = false;
    flags.c = a > (0xff - b);
    flags.h = ((a & 0x0f) + (b & 0x0f)) > 0x0f;

    uint8_t result = a + b;
    flags.z = (result == 0);
    return result;
}
uint8_t CPU::calc_adc8(uint8_t a, uint8_t b) {
    flags.n = false;
    flags.h = ((a & 0x0f) + (b & 0x0f) + flags.c) > 0x0f;

    uint8_t result = a + b + flags.c;
    flags.z = (result == 0);
    flags.c = (static_cast<uint16_t>(a) + b + flags.c) > 0xff;
    return result;
}
uint8_t CPU::calc_sub8(uint8_t a, uint8_t b) {
    flags.n = true;
    flags.c = a < b;                    // underflow
    flags.h = (a & 0x0f) < (b & 0x0f);  // borrow from bit4

    uint8_t result = a - b;
    flags.z = (result == 0);
    return result;
}
uint8_t CPU::calc_sbc8(uint8_t a, uint8_t b) {
    bool old_c = flags.c;
    flags.n = true;
    flags.h = (a & 0x0f) < ((b & 0x0f) + old_c);  // borrow from bit4

    uint8_t result = a - b - old_c;
    flags.z = (result == 0);
    flags.c = (static_cast<uint16_t>(b) + old_c) > a;  // underflow
    return result;
}
uint8_t CPU::calc_and8(uint8_t a, uint8_t b) {
    uint8_t result = a & b;
    set_flags_znhc(result == 0, false, true, false);
    return result;
}
uint8_t CPU::calc_xor8(uint8_t a, uint8_t b) {
    uint8_t result = a ^ b;
    set_flags_znhc(result == 0, false, false, false);
    return result;
}
uint8_t CPU::calc_or8(uint8_t a, uint8_t b) {
    uint8_t result = a | b;
    set_flags_znhc(result == 0, false, false, false);
    return result;
}

/* ------------------------ flags ----------------------- */
void CPU::set_flags_znhc(bool z, bool n, bool h, bool c) {
    flags.z = z;
    flags.n = n;
    flags.h = h;
    flags.c = c;
}
void CPU::set_flags_nhc(bool n, bool h, bool c) {
    flags.n = n;
    flags.h = h;
    flags.c = c;
}

/* ------------------- condition calls ------------------ */
clock_cycles CPU::ret_if(bool cc) {  // for RET cc
    if (!cc) return 2;
    ret();
    return 5;
}
clock_cycles CPU::jr_if(bool cc, uint8_t byte) {  // for JR cc
    if (!cc) return 2;
    data.signed_offset_PC(byte);
    return 3;
}
clock_cycles CPU::jp_if(bool cc, uint16_t addr) {  // for JR cc
    if (!cc) return 3;
    data.set_PC(addr);
    return 4;
}
clock_cycles CPU::call_if(bool cc, uint16_t addr) {  // for CALL cc
    if (!cc) return 3;
    call(addr);
    return 6;
}

/* ---------------------- SP manip ---------------------- */
void CPU::ret() {
    data.set_PC(read_mem16(data.SP));
    data.SP += 2;
    LOG_SP_LINE("Return to SP(0x" << data.SP << ") to PC(0x" << data.get_PC() << ")");
}
void CPU::call(uint16_t addr) {  // store data.PC in data.SP and jump to addr
    LOG_SP_LINE("Call from SP(0x" << data.SP << ") saving PC(0x" << data.get_PC() << ")");
    uint16_t ret_addr = data.get_PC();
    data.set_mem(--data.SP, ret_addr >> 8);    // MSB
    data.set_mem(--data.SP, ret_addr & 0xff);  // LSB
    data.set_PC(addr);                         // jp imm16
}
void CPU::add_SP(uint8_t byte) {  // adds as signed byte and sets flag
    flags.z = 0;
    flags.n = 0;
    flags.h = ((byte & 0x0f) + (data.SP & 0x0f)) > 0x0f;  // bit 3 overflow
    flags.c = (byte + (data.SP & 0xff)) > 0xff;           // bit 7 overflow
    data.SP = int8_t(byte) + data.SP;
}
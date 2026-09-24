#include "cpu.hpp"

// checks for any interrupts in the register 0xffff
uint16_t interrupt_addr[] = {VBlankInt, STATInt, TimerInt, SerialInt, JoypadInt};
void CPU::check_interrupt() {
    uint8_t IF = mm->read_mem(IF_ADDR);
    if (IF) {  // exits halt/stop states if interrupt enabled
        is_halted = false;
        is_stopped = false;
    }

    check_EI();
    if (!IME) return;
    uint8_t interrupts = mm->read_mem(IE_ADDR) & IF;
    if (interrupts == 0) return;
    IME = false;  // disable nested interrupts
    puts("int");

    // call handler, lowest bit has higher priority
    for (int idx = 0; idx <= 4; idx++) {
        if (!(interrupts & (1 << idx))) continue;  // no match

        // clear interrupt flag and call respective interrupt handler
        mm->set_mem(IF_ADDR, mm->read_mem(IF_ADDR) & ~(1 << idx));
        call(interrupt_addr[idx]);

        // printf("IF: %s\n", cvt_binary(data.read_mem(IF_ADDR)).c_str());

        return;
    }
}

void CPU::check_EI() {  // check for next instruction after EI to enable IME
    if (!set_EI) return;
    if (set_EI++ < 2) return;
    set_EI = 0;  // reset and enable IME
    IME = true;
}
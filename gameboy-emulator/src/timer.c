// timer.c - Timer functionality for Game Boy emulator
#include "gameboy.h"

typedef struct {
    uint16_t divider_cycles;
    uint16_t timer_cycles;
} Timer_State;

Timer_State timer_state;

void timer_init() {
    timer_state.divider_cycles = 0;
    timer_state.timer_cycles = 0;
    
    // Initialize timer registers
    memory_write(0xFF04, 0x00); // DIV
    memory_write(0xFF05, 0x00); // TIMA
    memory_write(0xFF06, 0x00); // TMA
    memory_write(0xFF07, 0x00); // TAC
}

void timer_update(uint16_t cycles) {
    // Update DIV register (increments at 16384 Hz)
    timer_state.divider_cycles += cycles;
    if (timer_state.divider_cycles >= 256) {
        timer_state.divider_cycles -= 256;
        uint8_t div = memory_read(0xFF04);
        memory_write(0xFF04, div + 1);
    }
    
    // Update timer if enabled
    uint8_t tac = memory_read(0xFF07);
    if (tac & 0x04) { // Timer enabled
        timer_state.timer_cycles += cycles;
        
        // Timer frequencies based on TAC bits 0-1
        uint16_t threshold;
        switch (tac & 0x03) {
            case 0: threshold = 1024; break; // 4096 Hz
            case 1: threshold = 16;   break; // 262144 Hz
            case 2: threshold = 64;   break; // 65536 Hz
            case 3: threshold = 256;  break; // 16384 Hz
        }
        
        if (timer_state.timer_cycles >= threshold) {
            timer_state.timer_cycles -= threshold;
            
            uint8_t tima = memory_read(0xFF05);
            if (tima == 0xFF) {
                // Timer overflow
                memory_write(0xFF05, memory_read(0xFF06)); // Reset to TMA
                // TODO: Trigger timer interrupt
            } else {
                memory_write(0xFF05, tima + 1);
            }
        }
    }
}

void update_timer() {
    timer_update(4); // Assume 4 cycles per instruction
}

void init_timer() {
    timer_init();
}
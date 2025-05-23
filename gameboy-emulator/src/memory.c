// memory.c - Memory management for Game Boy emulator
#include "gameboy.h"

uint8_t memory[MEMORY_SIZE];

void memory_init() {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory[i] = 0;
    }
}

uint8_t memory_read(uint16_t address) {
    // ROM area (0x0000-0x7FFF) - read from cartridge
    if (address < 0x8000) {
        return cartridge_read(address);
    }
    
    // Other areas read from memory array
    return memory[address];
}

void memory_write(uint16_t address, uint8_t value) {
    // ROM area is read-only, ignore writes
    if (address < 0x8000) {
        return;
    }
    
    // Echo RAM (0xE000-0xFDFF) mirrors WRAM (0xC000-0xDDFF)
    if (address >= 0xE000 && address < 0xFE00) {
        memory[address] = value;
        memory[address - 0x2000] = value; // Mirror to WRAM
        return;
    }
    
    memory[address] = value;
}

void init_memory() {
    memory_init();
}
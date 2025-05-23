// gameboy.h - Header file for the Game Boy emulator

#ifndef GAMEBOY_H
#define GAMEBOY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Memory map constants
#define MEMORY_SIZE 0x10000
#define ROM_BANK_SIZE 0x4000
#define VRAM_START 0x8000
#define VRAM_SIZE 0x2000
#define WRAM_START 0xC000
#define WRAM_SIZE 0x2000

// Function declarations
void init_memory();
void init_cpu();
void init_ppu();
void init_timer();
void init_input();
bool load_cartridge(const char* filename);
void execute_cpu_cycle();
void update_ppu();
void update_timer();
void handle_input();

// Memory functions
uint8_t memory_read(uint16_t address);
void memory_write(uint16_t address, uint8_t value);
void memory_init();

// CPU functions
void cpu_init();
void cpu_execute_instruction();
uint8_t cpu_fetch_byte();
uint16_t cpu_fetch_word();

// Timer functions
void timer_init();
void timer_update(uint16_t cycles);

// Input functions
void input_init();
void input_update();
uint8_t input_get_joypad();

// PPU functions
void ppu_init();
void ppu_update(uint16_t cycles);

// Cartridge functions
typedef struct {
    uint8_t *data;
    size_t size;
    char title[16];
    uint8_t type;
} Cartridge;

extern Cartridge *current_cartridge;
bool cartridge_load(const char *filename);
void cartridge_free();
uint8_t cartridge_read(uint16_t address);

// CPU state
typedef struct {
    uint8_t a, f, b, c, d, e, h, l;
    uint16_t pc, sp;
    bool halted;
    bool interrupts_enabled;
} CPU_State;

extern CPU_State cpu;

// Memory
extern uint8_t memory[MEMORY_SIZE];

// Flag register bits
#define FLAG_Z 0x80  // Zero flag
#define FLAG_N 0x40  // Negative flag
#define FLAG_H 0x20  // Half carry flag
#define FLAG_C 0x10  // Carry flag

#endif // GAMEBOY_H
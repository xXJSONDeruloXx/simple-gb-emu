// ppu.c - Picture Processing Unit (PPU) emulation
#include "gameboy.h"

// PPU registers (memory mapped)
#define LCDC 0xFF40  // LCD Control
#define STAT 0xFF41  // LCD Status
#define SCY  0xFF42  // Scroll Y
#define SCX  0xFF43  // Scroll X
#define LY   0xFF44  // LCD Y-Coordinate
#define LYC  0xFF45  // LY Compare

// PPU state
typedef struct {
    uint16_t cycles;
    uint8_t mode;
    uint8_t line;
} PPU_State;

PPU_State ppu;

void ppu_init() {
    ppu.cycles = 0;
    ppu.mode = 0;
    ppu.line = 0;
    
    // Initialize PPU registers
    memory_write(LCDC, 0x91); // LCD on, background on
    memory_write(STAT, 0x00);
    memory_write(SCY, 0x00);
    memory_write(SCX, 0x00);
    memory_write(LY, 0x00);
    memory_write(LYC, 0x00);
}

void ppu_update(uint16_t cycles) {
    // Check if LCD is enabled
    if (!(memory_read(LCDC) & 0x80)) {
        return;
    }
    
    ppu.cycles += cycles;
    
    // PPU modes:
    // Mode 0: H-Blank (204 cycles)
    // Mode 1: V-Blank (4560 cycles total)
    // Mode 2: OAM Scan (80 cycles)
    // Mode 3: Pixel Transfer (172 cycles)
    
    switch (ppu.mode) {
        case 0: // H-Blank
            if (ppu.cycles >= 204) {
                ppu.cycles -= 204;
                ppu.line++;
                
                if (ppu.line == 144) {
                    // Enter V-Blank
                    ppu.mode = 1;
                } else {
                    // Next line
                    ppu.mode = 2;
                }
                
                memory_write(LY, ppu.line);
            }
            break;
            
        case 1: // V-Blank
            if (ppu.cycles >= 456) {
                ppu.cycles -= 456;
                ppu.line++;
                
                if (ppu.line > 153) {
                    // Back to line 0
                    ppu.line = 0;
                    ppu.mode = 2;
                }
                
                memory_write(LY, ppu.line);
            }
            break;
            
        case 2: // OAM Scan
            if (ppu.cycles >= 80) {
                ppu.cycles -= 80;
                ppu.mode = 3;
            }
            break;
            
        case 3: // Pixel Transfer
            if (ppu.cycles >= 172) {
                ppu.cycles -= 172;
                ppu.mode = 0;
            }
            break;
    }
    
    // Update STAT register
    uint8_t stat = memory_read(STAT) & 0xF8;
    stat |= ppu.mode;
    memory_write(STAT, stat);
}

void update_ppu() {
    ppu_update(4); // Assume 4 cycles per instruction for simplicity
}

void init_ppu() {
    ppu_init();
}
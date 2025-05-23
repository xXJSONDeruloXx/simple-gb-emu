// input.c - Input handling for Game Boy emulator
#include "gameboy.h"

// Joypad register
#define JOYPAD_REG 0xFF00

// Button bits
#define BUTTON_A      0x01
#define BUTTON_B      0x02
#define BUTTON_SELECT 0x04
#define BUTTON_START  0x08
#define DPAD_RIGHT    0x01
#define DPAD_LEFT     0x02
#define DPAD_UP       0x04
#define DPAD_DOWN     0x08

typedef struct {
    uint8_t buttons;    // A, B, Select, Start
    uint8_t directions; // Right, Left, Up, Down
} Input_State;

Input_State input_state;

void input_init() {
    input_state.buttons = 0x0F;    // All buttons released (high)
    input_state.directions = 0x0F; // All directions released (high)
    
    // Initialize joypad register
    memory_write(JOYPAD_REG, 0xFF);
}

void input_update() {
    // In a real implementation, this would check for actual key presses
    // For now, we'll just maintain the current state
    
    uint8_t joypad = memory_read(JOYPAD_REG);
    
    // Check which button group is selected
    if (!(joypad & 0x10)) {
        // Direction keys selected
        joypad = (joypad & 0xF0) | input_state.directions;
    } else if (!(joypad & 0x20)) {
        // Button keys selected  
        joypad = (joypad & 0xF0) | input_state.buttons;
    } else {
        // No keys selected
        joypad |= 0x0F;
    }
    
    memory_write(JOYPAD_REG, joypad);
}

uint8_t input_get_joypad() {
    return memory_read(JOYPAD_REG);
}

void handle_input() {
    input_update();
}

void init_input() {
    input_init();
}
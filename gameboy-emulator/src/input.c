// input.c - Input handling for Game Boy emulator
#include "gameboy.h"
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>

// Terminal state to restore when exiting
static struct termios old_termios;

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
    
    // Set up terminal for non-canonical mode
    struct termios new_termios;
    
    // Save the current terminal settings
    tcgetattr(STDIN_FILENO, &old_termios);
    
    // Create new settings
    new_termios = old_termios;
    
    // Disable canonical mode and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 0;  // Return immediately, even if no chars
    new_termios.c_cc[VTIME] = 0; // No timeout
    
    // Set new terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    // Set stdin to non-blocking
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
}

void input_update() {
    // Check for key presses
    int ch = -1;
    
    // Non-blocking read from stdin
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    
    if (select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0) {
        ch = getchar();
        
        // Process the key press
        switch(ch) {
            // Arrow keys for D-pad
            case 'w': 
                input_state.directions &= ~DPAD_UP;    // Press Up
                break;
            case 's': 
                input_state.directions &= ~DPAD_DOWN;  // Press Down
                break;
            case 'a': 
                input_state.directions &= ~DPAD_LEFT;  // Press Left
                break;
            case 'd': 
                input_state.directions &= ~DPAD_RIGHT; // Press Right
                break;
            
            // Game buttons
            case 'j': 
                input_state.buttons &= ~BUTTON_B;      // Press B
                break;
            case 'k': 
                input_state.buttons &= ~BUTTON_A;      // Press A
                break;
            case 'n': 
                input_state.buttons &= ~BUTTON_SELECT; // Press Select
                break;
            case 'm': 
                input_state.buttons &= ~BUTTON_START;  // Press Start
                break;
            
            // Key release events (when user stops pressing a key)
            // In a more sophisticated implementation, we would track key up/down separately
            case 'W': 
                input_state.directions |= DPAD_UP;     // Release Up
                break;
            case 'S': 
                input_state.directions |= DPAD_DOWN;   // Release Down
                break;
            case 'A': 
                input_state.directions |= DPAD_LEFT;   // Release Left
                break;
            case 'D': 
                input_state.directions |= DPAD_RIGHT;  // Release Right
                break;
            case 'J': 
                input_state.buttons |= BUTTON_B;       // Release B
                break;
            case 'K': 
                input_state.buttons |= BUTTON_A;       // Release A
                break;
            case 'N': 
                input_state.buttons |= BUTTON_SELECT;  // Release Select
                break;
            case 'M': 
                input_state.buttons |= BUTTON_START;   // Release Start
                break;
        }
    }
    
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

void input_cleanup() {
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void init_input() {
    input_init();
}
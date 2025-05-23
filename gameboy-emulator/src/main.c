#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "gameboy.h"

// Game Boy screen dimensions (for terminal positioning)
#define GB_SCREEN_HEIGHT 144
#define GB_SCREEN_WIDTH 160

volatile bool running = true;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        // Reset terminal to normal state
        printf("\033[2J\033[H"); // Clear screen and move cursor to home
        printf("\033[?25h");     // Show cursor
        printf("\nShutting down emulator...\n");
        running = false;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <rom_file>\n", argv[0]);
        printf("Example: %s \"roms/Tetris (World) (Rev 1).gb\"\n", argv[0]);
        return 1;
    }

    // Set up signal handler for graceful shutdown
    signal(SIGINT, signal_handler);

    printf("Simple Game Boy Emulator\n");
    printf("Loading ROM: %s\n", argv[1]);

    // Initialize emulator components
    init_memory();
    init_cpu();
    init_ppu();
    init_timer();
    init_input();

    // Load the ROM
    if (!load_cartridge(argv[1])) {
        printf("Failed to load ROM: %s\n", argv[1]);
        return 1;
    }

    printf("ROM loaded successfully. Starting emulation...\n");
    printf("Press Ctrl+C to stop the emulator.\n");
    
    // Setup terminal for ASCII display
    printf("\033[2J");    // Clear the screen
    printf("\033[?25l");  // Hide cursor
    printf("\033[H");     // Move cursor to home position
    
    // Give user a moment to read the info before clearing screen
    sleep(1);

    // Main emulation loop
    uint64_t instruction_count = 0;
    uint64_t frames_rendered = 0;
    uint64_t last_status_update = 0;
    uint64_t last_frame_time = 0;
    const uint64_t FRAME_TIME_US = 16667; // ~60fps in microseconds
    
    // Setup for measuring time
    struct timeval tv;
    uint64_t current_time_us;
    
    gettimeofday(&tv, NULL);
    last_frame_time = tv.tv_sec * 1000000 + tv.tv_usec;
    
    while (running) {
        // Execute CPU instructions until a frame is ready
        while (!ppu_is_frame_ready() && running) {
            execute_cpu_cycle();
            instruction_count++;
            
            // Update PPU
            update_ppu();
            
            // Handle input
            handle_input();
            
            // Update timer
            update_timer();
            
            // Basic throttling to prevent runaway execution when debugging
            if (instruction_count % 5000 == 0) {
                usleep(1);
            }
        }
        
        if (!running) break;
        
        // Render the frame if ready
        if (ppu_is_frame_ready()) {
            frames_rendered++;
            
            // Display status info in the debug area (below the frame)
            if (instruction_count - last_status_update > 10000) {
                printf("\033[%dH", GB_SCREEN_HEIGHT + 2); // Move cursor below the frame
                printf("Frame: %llu | Instructions: %llu | PC: 0x%04X | A: 0x%02X | BC: 0x%02X%02X | DE: 0x%02X%02X | HL: 0x%02X%02X\n", 
                       frames_rendered, instruction_count, cpu.pc, cpu.a, cpu.b, cpu.c, cpu.d, cpu.e, cpu.h, cpu.l);
                last_status_update = instruction_count;
            }
            
            // Render the frame
            ppu_render_frame();
            
            // Frame timing to maintain 60fps
            gettimeofday(&tv, NULL);
            current_time_us = tv.tv_sec * 1000000 + tv.tv_usec;
            uint64_t elapsed = current_time_us - last_frame_time;
            
            if (elapsed < FRAME_TIME_US) {
                usleep(FRAME_TIME_US - elapsed);
            }
            
            gettimeofday(&tv, NULL);
            last_frame_time = tv.tv_sec * 1000000 + tv.tv_usec;
        }
    }

    // Cleanup
    cartridge_free();
    
    // Reset terminal to normal state
    printf("\033[2J\033[H"); // Clear screen and move cursor to home
    printf("\033[?25h");     // Show cursor
    
    printf("Emulator stopped. Total instructions executed: %llu\n", instruction_count);
    printf("Total frames rendered: %llu\n", frames_rendered);
    
    return 0;
}
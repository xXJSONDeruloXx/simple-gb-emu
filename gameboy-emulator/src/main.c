#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "gameboy.h"

volatile bool running = true;

void signal_handler(int sig) {
    if (sig == SIGINT) {
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

    // Main emulation loop
    uint64_t instruction_count = 0;
    while (running) {
        // Execute CPU instructions
        execute_cpu_cycle();
        instruction_count++;

        // Update PPU
        update_ppu();

        // Handle input
        handle_input();

        // Update timer
        update_timer();

        // Simple throttling - print status every 10000 instructions
        if (instruction_count % 10000 == 0) {
            printf("Instructions executed: %llu, PC: 0x%04X, A: 0x%02X\n", 
                   instruction_count, cpu.pc, cpu.a);
        }

        // Basic throttling to prevent runaway execution
        if (instruction_count % 1000 == 0) {
            usleep(1000); // Sleep for 1ms every 1000 instructions
        }
    }

    // Cleanup
    cartridge_free();
    printf("Emulator stopped. Total instructions executed: %llu\n", instruction_count);
    
    return 0;
}
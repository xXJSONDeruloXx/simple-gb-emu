// ppu.c - Picture Processing Unit (PPU) emulation
#include "gameboy.h"
#include <stdio.h>

// PPU registers (memory mapped)
#define LCDC 0xFF40  // LCD Control
#define STAT 0xFF41  // LCD Status
#define SCY  0xFF42  // Scroll Y
#define SCX  0xFF43  // Scroll X
#define LY   0xFF44  // LCD Y-Coordinate
#define LYC  0xFF45  // LY Compare
#define BGP  0xFF47  // Background Palette

// Game Boy screen resolution
#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144

// Tile data
#define TILE_DATA_TABLE_0 0x8000
#define TILE_DATA_TABLE_1 0x8800
#define TILE_DATA_TABLE_2 0x9000

// Background tile maps
#define BG_MAP_0 0x9800
#define BG_MAP_1 0x9C00

// PPU state
typedef struct {
    uint16_t cycles;
    uint8_t mode;
    uint8_t line;
    // ASCII framebuffer - 2 chars represent 1 pixel for better proportions
    char framebuffer[GB_SCREEN_HEIGHT][GB_SCREEN_WIDTH*2 + 1]; // +1 for null terminator
    bool frame_ready;
    int frame_count;
} PPU_State;

PPU_State ppu;

// ASCII characters for different brightness levels - using only standard ASCII
const char* pixel_chars[] = {
    "  ", // 0 - White (empty space)
    "..", // 1 - Light gray (using dots)
    "##", // 2 - Dark gray (using hash)
    "@@"  // 3 - Black (using at symbols)
};

void ppu_init() {
    ppu.cycles = 0;
    ppu.mode = 0;
    ppu.line = 0;
    ppu.frame_ready = false;
    ppu.frame_count = 0;
    
    // Clear framebuffer
    for (int y = 0; y < GB_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < GB_SCREEN_WIDTH; x++) {
            ppu.framebuffer[y][x*2] = ' ';
            ppu.framebuffer[y][x*2+1] = ' ';
        }
        ppu.framebuffer[y][GB_SCREEN_WIDTH*2] = '\0';
    }
    
    // Initialize PPU registers
    memory_write(LCDC, 0x91); // LCD on, background on
    memory_write(STAT, 0x00);
    memory_write(SCY, 0x00);
    memory_write(SCX, 0x00);
    memory_write(LY, 0x00);
    memory_write(LYC, 0x00);
    memory_write(BGP, 0xFC); // Default palette: 11 10 01 00 (3,2,1,0)
}

// Helper function to get the color from a pixel
uint8_t get_color(uint8_t palette, uint8_t color_id) {
    // Extract the color from the palette based on color_id (0-3)
    return (palette >> (color_id * 2)) & 0x03;
}

// Render a single scanline
void render_scanline(uint8_t line) {
    uint8_t lcdc = memory_read(LCDC);
    
    // Check if LCD and background are enabled
    if (!(lcdc & 0x80) || !(lcdc & 0x01)) {
        return;
    }
    
    uint8_t scroll_y = memory_read(SCY);
    uint8_t scroll_x = memory_read(SCX);
    uint8_t palette = memory_read(BGP);
    
    // Determine which background map to use
    uint16_t bg_map_addr = (lcdc & 0x08) ? BG_MAP_1 : BG_MAP_0;
    
    // Determine which tile data table to use
    bool tile_data_unsigned = (lcdc & 0x10) != 0;
    uint16_t tile_data_base = tile_data_unsigned ? TILE_DATA_TABLE_0 : TILE_DATA_TABLE_2;
    
    // Calculate the y position within the entire background
    uint8_t bg_y = (line + scroll_y) & 0xFF;
    
    // Get the tile row (which 8-pixel row of the background)
    uint8_t tile_row = bg_y / 8;
    
    // Get the pixel row within the tile (0-7)
    uint8_t tile_y = bg_y % 8;
    
    // Render each pixel in the scanline
    for (int pixel = 0; pixel < GB_SCREEN_WIDTH; pixel++) {
        // Calculate the x position within the entire background
        uint8_t bg_x = (pixel + scroll_x) & 0xFF;
        
        // Get the tile column
        uint8_t tile_col = bg_x / 8;
        
        // Get the pixel column within the tile (0-7)
        uint8_t tile_x = bg_x % 8;
        
        // Get the tile number from the tile map
        uint16_t tile_map_addr = bg_map_addr + (tile_row * 32) + tile_col;
        uint8_t tile_num = memory_read(tile_map_addr);
        
        // Calculate the actual tile data address
        uint16_t tile_addr;
        if (tile_data_unsigned) {
            tile_addr = tile_data_base + (tile_num * 16);
        } else {
            // Signed addressing, tile_num is treated as signed
            tile_addr = tile_data_base + ((int8_t)(tile_num) + 128) * 16;
        }
        
        // Each tile has 8 rows, each row is 2 bytes
        uint16_t line_addr = tile_addr + (tile_y * 2);
        
        // Get the 2 bytes that define the 8 pixels in this row
        uint8_t low_byte = memory_read(line_addr);
        uint8_t high_byte = memory_read(line_addr + 1);
        
        // Combine bits to get the color ID for this pixel (0-3)
        uint8_t bit_position = 7 - tile_x; // Pixels are stored MSB-first
        uint8_t color_id = ((high_byte >> bit_position) & 0x01) << 1;
        color_id |= (low_byte >> bit_position) & 0x01;
        
        // Convert color ID to actual color using the palette
        uint8_t color = get_color(palette, color_id);
        
        // Set the pixel in the framebuffer using our improved ASCII representation
        const char* pixel_ascii = pixel_chars[color];
        ppu.framebuffer[line][pixel*2] = pixel_ascii[0];
        ppu.framebuffer[line][pixel*2+1] = pixel_ascii[1];
    }
}

void ppu_render_frame() {
    // Clear terminal and move cursor to top
    printf("\033[2J\033[H");
    
    ppu.frame_count++;
    
    // Title bar
    printf("+------------ SIMPLE GAME BOY EMULATOR ------------+\n");
    
    // Create a horizontal border above the screen
    printf("+");
    for (int i = 0; i < GB_SCREEN_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");
    
    // Display the framebuffer with borders
    for (int y = 0; y < GB_SCREEN_HEIGHT; y++) {
        // Skip alternate rows for better aspect ratio
        if (y % 2 == 0) {
            printf("|");
            for (int x = 0; x < GB_SCREEN_WIDTH; x++) {
                // Get the 2 chars representing this pixel
                char c1 = ppu.framebuffer[y][x*2];
                char c2 = ppu.framebuffer[y][x*2+1];
                
                // Map the characters to the corresponding brightness level
                int brightness_level = 0;
                if (c1 == ' ' && c2 == ' ') brightness_level = 0;      // White
                else if (c1 == '.' && c2 == '.') brightness_level = 1; // Light gray
                else if (c1 == '#' && c2 == '#') brightness_level = 2; // Dark gray
                else if (c1 == '@' && c2 == '@') brightness_level = 3; // Black
                
                // Print a single char based on brightness level for better density
                switch(brightness_level) {
                    case 0: putchar(' '); break;  // White - empty space
                    case 1: putchar('.'); break;  // Light gray - dot
                    case 2: putchar('#'); break;  // Dark gray - hash
                    case 3: putchar('@'); break;  // Black - at symbol
                    default: putchar('?'); break; // Shouldn't happen
                }
            }
            printf("|\n");
        }
    }
    
    // Create a horizontal border below the screen
    printf("+");
    for (int i = 0; i < GB_SCREEN_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");
    
    // Debug information
    printf("+------------------ DEBUG INFO -------------------+\n");
    printf("| Frame: %-6d                                   |\n", ppu.frame_count);
    printf("| Mode:  %-6d                                   |\n", ppu.mode);
    printf("| Line:  %-6d                                   |\n", ppu.line);
    printf("+------------------------------------------------+\n");
    
    // Reset frame_ready flag
    ppu.frame_ready = false;
}

void ppu_update(uint16_t cycles) {
    // Add cycles to PPU counter
    ppu.cycles += cycles;
    
    // LCD is disabled, don't update
    if (!(memory_read(LCDC) & 0x80)) {
        return;
    }
    
    switch (ppu.mode) {
        case 0: // H-Blank
            if (ppu.cycles >= 204) {
                ppu.cycles -= 204;
                ppu.line++;
                memory_write(LY, ppu.line);
                
                // Render the completed scanline
                render_scanline(ppu.line - 1);
                
                if (ppu.line == 144) {
                    // Enter V-Blank
                    ppu.mode = 1;
                    ppu.frame_ready = true; // Set frame ready flag
                } else {
                    // Next line
                    ppu.mode = 2;
                }
            }
            break;
            
        case 1: // V-Blank
            if (ppu.cycles >= 456) {
                ppu.cycles -= 456;
                ppu.line++;
                memory_write(LY, ppu.line);
                
                if (ppu.line > 153) {
                    // Back to line 0
                    ppu.line = 0;
                    memory_write(LY, 0);
                    ppu.mode = 2;
                }
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

// Check if a frame is ready to be displayed
bool ppu_is_frame_ready() {
    return ppu.frame_ready;
}

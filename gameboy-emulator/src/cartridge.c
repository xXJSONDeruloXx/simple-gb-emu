// cartridge.c - ROM cartridge handling
#include "gameboy.h"

Cartridge *current_cartridge = NULL;

bool cartridge_load(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open ROM file: %s\n", filename);
        return false;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate cartridge
    current_cartridge = malloc(sizeof(Cartridge));
    if (!current_cartridge) {
        fclose(file);
        printf("Failed to allocate memory for cartridge\n");
        return false;
    }

    // Allocate ROM data
    current_cartridge->data = malloc(size);
    if (!current_cartridge->data) {
        free(current_cartridge);
        current_cartridge = NULL;
        fclose(file);
        printf("Failed to allocate memory for ROM data\n");
        return false;
    }

    // Read ROM data
    if (fread(current_cartridge->data, 1, size, file) != size) {
        cartridge_free();
        fclose(file);
        printf("Failed to read ROM data\n");
        return false;
    }

    fclose(file);
    current_cartridge->size = size;

    // Extract cartridge info
    if (size >= 0x150) {
        // Copy title (0x134-0x143)
        for (int i = 0; i < 15; i++) {
            current_cartridge->title[i] = current_cartridge->data[0x134 + i];
            if (current_cartridge->title[i] == 0) break;
        }
        current_cartridge->title[15] = '\0';
        
        // Get cartridge type
        current_cartridge->type = current_cartridge->data[0x147];
        
        printf("Loaded ROM: %s (Type: 0x%02X)\n", current_cartridge->title, current_cartridge->type);
    }

    return true;
}

void cartridge_free() {
    if (current_cartridge) {
        if (current_cartridge->data) {
            free(current_cartridge->data);
        }
        free(current_cartridge);
        current_cartridge = NULL;
    }
}

uint8_t cartridge_read(uint16_t address) {
    if (!current_cartridge || address >= current_cartridge->size) {
        return 0xFF; // Return 0xFF for unmapped reads
    }
    return current_cartridge->data[address];
}

bool load_cartridge(const char* filename) {
    return cartridge_load(filename);
}
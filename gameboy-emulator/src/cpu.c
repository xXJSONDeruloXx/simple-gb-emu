// cpu.c - Game Boy CPU (Sharp LR35902) emulation
#include "gameboy.h"

CPU_State cpu;

void cpu_init() {
    // Initial register values for Game Boy after boot ROM
    cpu.a = 0x01;
    cpu.f = 0xB0;
    cpu.b = 0x00;
    cpu.c = 0x13;
    cpu.d = 0x00;
    cpu.e = 0xD8;
    cpu.h = 0x01;
    cpu.l = 0x4D;
    cpu.pc = 0x0100;  // Start after boot ROM
    cpu.sp = 0xFFFE;
    cpu.halted = false;
    cpu.interrupts_enabled = false;
}

uint8_t cpu_fetch_byte() {
    return memory_read(cpu.pc++);
}

uint16_t cpu_fetch_word() {
    uint8_t low = cpu_fetch_byte();
    uint8_t high = cpu_fetch_byte();
    return (high << 8) | low;
}

// Helper functions for register pairs
uint16_t get_bc() { return (cpu.b << 8) | cpu.c; }
uint16_t get_de() { return (cpu.d << 8) | cpu.e; }
uint16_t get_hl() { return (cpu.h << 8) | cpu.l; }

void set_bc(uint16_t value) { cpu.b = value >> 8; cpu.c = value & 0xFF; }
void set_de(uint16_t value) { cpu.d = value >> 8; cpu.e = value & 0xFF; }
void set_hl(uint16_t value) { cpu.h = value >> 8; cpu.l = value & 0xFF; }

// Flag operations
void set_flag(uint8_t flag) { cpu.f |= flag; }
void clear_flag(uint8_t flag) { cpu.f &= ~flag; }
bool get_flag(uint8_t flag) { return (cpu.f & flag) != 0; }

void cpu_execute_instruction() {
    if (cpu.halted) {
        return; // CPU is halted, do nothing
    }

    uint8_t opcode = cpu_fetch_byte();
    
    switch (opcode) {
        case 0x00: // NOP
            break;
            
        case 0x01: // LD BC, d16
            set_bc(cpu_fetch_word());
            break;
            
        case 0x02: // LD (BC), A
            memory_write(get_bc(), cpu.a);
            break;
            
        case 0x03: // INC BC
            set_bc(get_bc() + 1);
            break;
            
        case 0x04: // INC B
            cpu.b++;
            cpu.f = (cpu.f & FLAG_C) | (cpu.b == 0 ? FLAG_Z : 0) | ((cpu.b & 0x0F) == 0 ? FLAG_H : 0);
            break;
            
        case 0x05: // DEC B
            cpu.f = (cpu.f & FLAG_C) | FLAG_N | ((cpu.b & 0x0F) == 0 ? FLAG_H : 0);
            cpu.b--;
            if (cpu.b == 0) cpu.f |= FLAG_Z;
            break;
            
        case 0x06: // LD B, d8
            cpu.b = cpu_fetch_byte();
            break;
            
        case 0x0A: // LD A, (BC)
            cpu.a = memory_read(get_bc());
            break;
            
        case 0x0B: // DEC BC
            set_bc(get_bc() - 1);
            break;
            
        case 0x0C: // INC C
            cpu.c++;
            cpu.f = (cpu.f & FLAG_C) | (cpu.c == 0 ? FLAG_Z : 0) | ((cpu.c & 0x0F) == 0 ? FLAG_H : 0);
            break;
            
        case 0x0D: // DEC C
            cpu.f = (cpu.f & FLAG_C) | FLAG_N | ((cpu.c & 0x0F) == 0 ? FLAG_H : 0);
            cpu.c--;
            if (cpu.c == 0) cpu.f |= FLAG_Z;
            break;
            
        case 0x0E: // LD C, d8
            cpu.c = cpu_fetch_byte();
            break;
            
        case 0x0F: // RRCA (Rotate Right Circular Accumulator)
            cpu.f = (cpu.a & 0x01) ? FLAG_C : 0;
            cpu.a = (cpu.a >> 1) | ((cpu.f & FLAG_C) ? 0x80 : 0);
            break;
            
        case 0x11: // LD DE, d16
            set_de(cpu_fetch_word());
            break;
            
        case 0x12: // LD (DE), A
            memory_write(get_de(), cpu.a);
            break;
            
        case 0x13: // INC DE
            set_de(get_de() + 1);
            break;
            
        case 0x16: // LD D, d8
            cpu.d = cpu_fetch_byte();
            break;
            
        case 0x19: // ADD HL, DE
        {
            uint32_t result = get_hl() + get_de();
            cpu.f = (cpu.f & FLAG_Z) | ((result > 0xFFFF) ? FLAG_C : 0) | 
                   (((get_hl() & 0x0FFF) + (get_de() & 0x0FFF)) > 0x0FFF ? FLAG_H : 0);
            set_hl(result & 0xFFFF);
            break;
        }
        
        case 0x1A: // LD A, (DE)
            cpu.a = memory_read(get_de());
            break;
            
        case 0x1B: // DEC DE
            set_de(get_de() - 1);
            break;
            
        case 0x1C: // INC E
            cpu.e++;
            cpu.f = (cpu.f & FLAG_C) | (cpu.e == 0 ? FLAG_Z : 0) | ((cpu.e & 0x0F) == 0 ? FLAG_H : 0);
            break;
            
        case 0x1D: // DEC E
            cpu.f = (cpu.f & FLAG_C) | FLAG_N | ((cpu.e & 0x0F) == 0 ? FLAG_H : 0);
            cpu.e--;
            if (cpu.e == 0) cpu.f |= FLAG_Z;
            break;
            
        case 0x1E: // LD E, d8
            cpu.e = cpu_fetch_byte();
            break;
            
        case 0x18: // JR r8 (relative jump)
        {
            int8_t offset = (int8_t)cpu_fetch_byte();
            cpu.pc += offset;
            break;
        }
        
        case 0x20: // JR NZ, r8
        {
            int8_t offset = (int8_t)cpu_fetch_byte();
            if (!get_flag(FLAG_Z)) {
                cpu.pc += offset;
            }
            break;
        }
        
        case 0x21: // LD HL, d16
            set_hl(cpu_fetch_word());
            break;
            
        case 0x22: // LD (HL+), A
            memory_write(get_hl(), cpu.a);
            set_hl(get_hl() + 1);
            break;
            
        case 0x23: // INC HL
            set_hl(get_hl() + 1);
            break;
            
        case 0x26: // LD H, d8
            cpu.h = cpu_fetch_byte();
            break;
            
        case 0x28: // JR Z, r8
        {
            int8_t offset = (int8_t)cpu_fetch_byte();
            if (get_flag(FLAG_Z)) {
                cpu.pc += offset;
            }
            break;
        }
        
        case 0x2A: // LD A, (HL+)
            cpu.a = memory_read(get_hl());
            set_hl(get_hl() + 1);
            break;
            
        case 0x2F: // CPL (Complement A)
            cpu.a = ~cpu.a;
            cpu.f |= FLAG_N | FLAG_H;
            break;
        
        case 0x31: // LD SP, d16
            cpu.sp = cpu_fetch_word();
            break;
            
        case 0x32: // LD (HL-), A
            memory_write(get_hl(), cpu.a);
            set_hl(get_hl() - 1);
            break;
            
        case 0x36: // LD (HL), d8
        {
            uint8_t value = cpu_fetch_byte();
            memory_write(get_hl(), value);
            break;
        }
        
        case 0x37: // SCF (Set Carry Flag)
            cpu.f = (cpu.f & FLAG_Z) | FLAG_C;
            break;
            
        case 0x3D: // DEC A
            cpu.f = (cpu.f & FLAG_C) | FLAG_N | ((cpu.a & 0x0F) == 0 ? FLAG_H : 0);
            cpu.a--;
            if (cpu.a == 0) cpu.f |= FLAG_Z;
            break;
            
        case 0x3E: // LD A, d8
            cpu.a = cpu_fetch_byte();
            break;
            
        case 0x44: // LD B, H
            cpu.b = cpu.h;
            break;
            
        case 0x47: // LD B, A
            cpu.b = cpu.a;
            break;
            
        case 0x4F: // LD C, A
            cpu.c = cpu.a;
            break;
            
        case 0x56: // LD D, (HL)
            cpu.d = memory_read(get_hl());
            break;
            
        case 0x57: // LD D, A
            cpu.d = cpu.a;
            break;
            
        case 0x5E: // LD E, (HL)
            cpu.e = memory_read(get_hl());
            break;
            
        case 0x5F: // LD E, A
            cpu.e = cpu.a;
            break;
            
        case 0x67: // LD H, A
            cpu.h = cpu.a;
            break;
            
        case 0x6F: // LD L, A
            cpu.l = cpu.a;
            break;
            
        case 0x77: // LD (HL), A
            memory_write(get_hl(), cpu.a);
            break;
            
        case 0x78: // LD A, B
            cpu.a = cpu.b;
            break;
            
        case 0x79: // LD A, C
            cpu.a = cpu.c;
            break;
            
        case 0x7A: // LD A, D
            cpu.a = cpu.d;
            break;
            
        case 0x7B: // LD A, E
            cpu.a = cpu.e;
            break;
            
        case 0x7C: // LD A, H
            cpu.a = cpu.h;
            break;
            
        case 0x7D: // LD A, L
            cpu.a = cpu.l;
            break;
            
        case 0x7E: // LD A, (HL)
            cpu.a = memory_read(get_hl());
            break;
            
        case 0x7F: // LD A, A (essentially NOP)
            break;
            
        case 0x87: // ADD A, A
        {
            uint16_t result = cpu.a + cpu.a;
            cpu.f = ((result & 0xFF) == 0) ? FLAG_Z : 0;
            cpu.f |= (result > 0xFF) ? FLAG_C : 0;
            cpu.f |= ((cpu.a & 0x0F) + (cpu.a & 0x0F) > 0x0F) ? FLAG_H : 0;
            cpu.a = result & 0xFF;
            break;
        }
        
        case 0xA1: // AND C
            cpu.a &= cpu.c;
            cpu.f = (cpu.a == 0 ? FLAG_Z : 0) | FLAG_H;
            break;
            
        case 0xA7: // AND A
            cpu.f = (cpu.a == 0 ? FLAG_Z : 0) | FLAG_H;
            break;
            
        case 0xA9: // XOR C
            cpu.a ^= cpu.c;
            cpu.f = (cpu.a == 0) ? FLAG_Z : 0;
            break;
            
        case 0xAE: // XOR (HL)
            cpu.a ^= memory_read(get_hl());
            cpu.f = (cpu.a == 0) ? FLAG_Z : 0;
            break;
            
        case 0xAF: // XOR A
            cpu.a = 0;
            cpu.f = FLAG_Z;
            break;
            
        case 0xB0: // OR B
            cpu.a |= cpu.b;
            cpu.f = (cpu.a == 0) ? FLAG_Z : 0;
            break;
            
        case 0xB1: // OR C
            cpu.a |= cpu.c;
            cpu.f = (cpu.a == 0) ? FLAG_Z : 0;
            break;
            
        case 0xB7: // OR A
            cpu.f = (cpu.a == 0) ? FLAG_Z : 0;
            break;
            
        case 0xBE: // CP (HL)
        {
            uint8_t value = memory_read(get_hl());
            uint8_t result = cpu.a - value;
            cpu.f = FLAG_N | (result == 0 ? FLAG_Z : 0) | 
                   ((cpu.a & 0x0F) < (value & 0x0F) ? FLAG_H : 0) |
                   (cpu.a < value ? FLAG_C : 0);
            break;
        }
            
        case 0xC0: // RET NZ
            if (!get_flag(FLAG_Z)) {
                uint8_t low = memory_read(cpu.sp++);
                uint8_t high = memory_read(cpu.sp++);
                cpu.pc = (high << 8) | low;
            }
            break;
            
        case 0xC1: // POP BC
        {
            uint8_t low = memory_read(cpu.sp++);
            uint8_t high = memory_read(cpu.sp++);
            set_bc((high << 8) | low);
            break;
        }
        
        case 0xC2: // JP NZ, a16
        {
            uint16_t addr = cpu_fetch_word();
            if (!get_flag(FLAG_Z)) {
                cpu.pc = addr;
            }
            break;
        }
        
        case 0xC3: // JP a16
            cpu.pc = cpu_fetch_word();
            break;
            
        case 0xC5: // PUSH BC
            cpu.sp--;
            memory_write(cpu.sp, cpu.b);
            cpu.sp--;
            memory_write(cpu.sp, cpu.c);
            break;
            
        case 0xC6: // ADD A, d8
        {
            uint8_t value = cpu_fetch_byte();
            uint16_t result = cpu.a + value;
            
            cpu.f = ((result & 0xFF) == 0) ? FLAG_Z : 0;
            cpu.f |= (result > 0xFF) ? FLAG_C : 0;
            cpu.f |= ((cpu.a & 0x0F) + (value & 0x0F) > 0x0F) ? FLAG_H : 0;
            
            cpu.a = result & 0xFF;
            break;
        }
            
        case 0xC9: // RET
        {
            // Pop return address from stack
            uint8_t low = memory_read(cpu.sp++);
            uint8_t high = memory_read(cpu.sp++);
            cpu.pc = (high << 8) | low;
            break;
        }
            
        case 0xCB: // CB prefix instructions
        {
            uint8_t cb_opcode = cpu_fetch_byte();
            switch (cb_opcode) {
                case 0x37: // SWAP A
                    cpu.a = ((cpu.a & 0x0F) << 4) | ((cpu.a & 0xF0) >> 4);
                    cpu.f = (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                    
                case 0x7C: // BIT 7, H
                    cpu.f = (cpu.f & FLAG_C) | FLAG_H | ((cpu.h & 0x80) ? 0 : FLAG_Z);
                    break;
                    
                default:
                    printf("Unimplemented CB opcode: 0x%02X at PC: 0x%04X\n", cb_opcode, cpu.pc - 2);
                    break;
            }
            break;
        }
        
        case 0xCD: // CALL a16
        {
            uint16_t addr = cpu_fetch_word();
            // Push return address onto stack
            cpu.sp--;
            memory_write(cpu.sp, cpu.pc >> 8);
            cpu.sp--;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            cpu.pc = addr;
            break;
        }
        
        case 0xCE: // ADC A, d8
        {
            uint8_t value = cpu_fetch_byte();
            uint8_t carry = get_flag(FLAG_C) ? 1 : 0;
            uint16_t result = cpu.a + value + carry;
            
            cpu.f = (result & 0xFF) == 0 ? FLAG_Z : 0;
            cpu.f |= (result > 0xFF) ? FLAG_C : 0;
            cpu.f |= ((cpu.a & 0x0F) + (value & 0x0F) + carry > 0x0F) ? FLAG_H : 0;
            
            cpu.a = result & 0xFF;
            break;
        }
        
        case 0xD5: // PUSH DE
            cpu.sp--;
            memory_write(cpu.sp, cpu.d);
            cpu.sp--;
            memory_write(cpu.sp, cpu.e);
            break;
            
        case 0xE0: // LDH (a8), A
        {
            uint8_t addr = cpu_fetch_byte();
            memory_write(0xFF00 + addr, cpu.a);
            break;
        }
        
        case 0xE1: // POP HL
        {
            uint8_t low = memory_read(cpu.sp++);
            uint8_t high = memory_read(cpu.sp++);
            set_hl((high << 8) | low);
            break;
        }
        
        case 0xE2: // LD (C), A
            memory_write(0xFF00 + cpu.c, cpu.a);
            break;
            
        case 0xE5: // PUSH HL
            cpu.sp--;
            memory_write(cpu.sp, cpu.h);
            cpu.sp--;
            memory_write(cpu.sp, cpu.l);
            break;
        
        case 0xE6: // AND d8
        {
            uint8_t value = cpu_fetch_byte();
            cpu.a &= value;
            cpu.f = (cpu.a == 0 ? FLAG_Z : 0) | FLAG_H;
            break;
        }
        
        case 0xE9: // JP (HL)
            cpu.pc = get_hl();
            break;
            
        case 0xEA: // LD (a16), A
        {
            uint16_t addr = cpu_fetch_word();
            memory_write(addr, cpu.a);
            break;
        }
        
        case 0xEF: // RST 28H
            // Push return address onto stack
            cpu.sp--;
            memory_write(cpu.sp, cpu.pc >> 8);
            cpu.sp--;
            memory_write(cpu.sp, cpu.pc & 0xFF);
            cpu.pc = 0x0028;
            break;
        
        case 0xF0: // LDH A, (a8)
        {
            uint8_t addr = cpu_fetch_byte();
            cpu.a = memory_read(0xFF00 + addr);
            break;
        }
        
        case 0xF2: // LD A, (C)
            cpu.a = memory_read(0xFF00 + cpu.c);
            break;
        
        case 0xF3: // DI (Disable Interrupts)
            cpu.interrupts_enabled = false;
            break;
            
        case 0xF5: // PUSH AF
            cpu.sp--;
            memory_write(cpu.sp, cpu.a);
            cpu.sp--;
            memory_write(cpu.sp, cpu.f);
            break;
            
        case 0xFA: // LD A, (a16)
        {
            uint16_t addr = cpu_fetch_word();
            cpu.a = memory_read(addr);
            break;
        }
        
        case 0xFB: // EI (Enable Interrupts)
            cpu.interrupts_enabled = true;
            break;
        
        case 0xFE: // CP d8
        {
            uint8_t value = cpu_fetch_byte();
            uint8_t result = cpu.a - value;
            cpu.f = FLAG_N | (result == 0 ? FLAG_Z : 0) | 
                   ((cpu.a & 0x0F) < (value & 0x0F) ? FLAG_H : 0) |
                   (cpu.a < value ? FLAG_C : 0);
            break;
        }
        
        case 0x76: // HALT
            cpu.halted = true;
            break;
            
        default:
            printf("Unimplemented opcode: 0x%02X at PC: 0x%04X\n", opcode, cpu.pc - 1);
            break;
    }
}

void execute_cpu_cycle() {
    cpu_execute_instruction();
}

void init_cpu() {
    cpu_init();
}
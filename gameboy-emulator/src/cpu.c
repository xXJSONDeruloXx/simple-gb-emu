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
            
        case 0x07: // RLCA (Rotate Left Circular Accumulator)
            cpu.f = (cpu.a & 0x80) ? FLAG_C : 0;
            cpu.a = (cpu.a << 1) | ((cpu.f & FLAG_C) ? 0x01 : 0);
            break;
            
        case 0x08: // LD (a16), SP
        {
            uint16_t addr = cpu_fetch_word();
            memory_write(addr, cpu.sp & 0xFF);
            memory_write(addr + 1, cpu.sp >> 8);
            break;
        }
        
        case 0x09: // ADD HL, BC
        {
            uint32_t result = get_hl() + get_bc();
            cpu.f = (cpu.f & FLAG_Z) | ((result > 0xFFFF) ? FLAG_C : 0) | 
                   (((get_hl() & 0x0FFF) + (get_bc() & 0x0FFF)) > 0x0FFF ? FLAG_H : 0);
            set_hl(result & 0xFFFF);
            break;
        }
        
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
            
        case 0x14: // INC D
            cpu.d++;
            cpu.f = (cpu.f & FLAG_C) | (cpu.d == 0 ? FLAG_Z : 0) | ((cpu.d & 0x0F) == 0 ? FLAG_H : 0);
            break;
            
        case 0x15: // DEC D
            cpu.f = (cpu.f & FLAG_C) | FLAG_N | ((cpu.d & 0x0F) == 0 ? FLAG_H : 0);
            cpu.d--;
            if (cpu.d == 0) cpu.f |= FLAG_Z;
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
            
        case 0x17: // RLA (Rotate Left through Carry)
        {
            uint8_t old_carry = get_flag(FLAG_C) ? 1 : 0;
            cpu.f = (cpu.a & 0x80) ? FLAG_C : 0;
            cpu.a = (cpu.a << 1) | old_carry;
            break;
        }
        
        case 0x18: // JR r8 (relative jump)
        {
            int8_t offset = (int8_t)cpu_fetch_byte();
            cpu.pc += offset;
            break;
        }
        
        case 0x1F: // RRA (Rotate Right through Carry)
        {
            uint8_t old_carry = get_flag(FLAG_C) ? 0x80 : 0;
            cpu.f = (cpu.a & 0x01) ? FLAG_C : 0;
            cpu.a = (cpu.a >> 1) | old_carry;
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
            
        case 0x24: // INC H
            cpu.h++;
            cpu.f = (cpu.f & FLAG_C) | (cpu.h == 0 ? FLAG_Z : 0) | ((cpu.h & 0x0F) == 0 ? FLAG_H : 0);
            break;
            
        case 0x25: // DEC H
            cpu.f = (cpu.f & FLAG_C) | FLAG_N | ((cpu.h & 0x0F) == 0 ? FLAG_H : 0);
            cpu.h--;
            if (cpu.h == 0) cpu.f |= FLAG_Z;
            break;
            
        case 0x26: // LD H, d8
            cpu.h = cpu_fetch_byte();
            break;
            
        case 0x27: // DAA (Decimal Adjust Accumulator)
        {
            uint8_t correction = 0;
            bool set_carry = false;
            
            if (get_flag(FLAG_H) || (!get_flag(FLAG_N) && (cpu.a & 0x0F) > 9)) {
                correction |= 0x06;
            }
            
            if (get_flag(FLAG_C) || (!get_flag(FLAG_N) && cpu.a > 0x99)) {
                correction |= 0x60;
                set_carry = true;
            }
            
            if (get_flag(FLAG_N)) {
                cpu.a -= correction;
            } else {
                cpu.a += correction;
            }
            
            cpu.f = (cpu.f & FLAG_N) | (cpu.a == 0 ? FLAG_Z : 0);
            if (set_carry) cpu.f |= FLAG_C;
            break;
        }
        
        case 0x28: // JR Z, r8
        {
            int8_t offset = (int8_t)cpu_fetch_byte();
            if (get_flag(FLAG_Z)) {
                cpu.pc += offset;
            }
            break;
        }
        
        case 0x29: // ADD HL, HL
        {
            uint32_t result = get_hl() + get_hl();
            cpu.f = (cpu.f & FLAG_Z) | ((result > 0xFFFF) ? FLAG_C : 0) | 
                   (((get_hl() & 0x0FFF) + (get_hl() & 0x0FFF)) > 0x0FFF ? FLAG_H : 0);
            set_hl(result & 0xFFFF);
            break;
        }
        
        case 0x2A: // LD A, (HL+)
            cpu.a = memory_read(get_hl());
            set_hl(get_hl() + 1);
            break;
            
        case 0x2C: // INC L
            cpu.l++;
            cpu.f = (cpu.f & FLAG_C) | (cpu.l == 0 ? FLAG_Z : 0) | ((cpu.l & 0x0F) == 0 ? FLAG_H : 0);
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
            
        case 0x35: // DEC (HL)
        {
            uint8_t value = memory_read(get_hl());
            cpu.f = (cpu.f & FLAG_C) | FLAG_N | ((value & 0x0F) == 0 ? FLAG_H : 0);
            value--;
            if (value == 0) cpu.f |= FLAG_Z;
            memory_write(get_hl(), value);
            break;
        }
        
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
        
        case 0xC8: // RET Z
            if (get_flag(FLAG_Z)) {
                uint8_t low = memory_read(cpu.sp++);
                uint8_t high = memory_read(cpu.sp++);
                cpu.pc = (high << 8) | low;
            }
            break;
            
        case 0xCA: // JP Z, a16
        {
            uint16_t addr = cpu_fetch_word();
            if (get_flag(FLAG_Z)) {
                cpu.pc = addr;
            }
            break;
        }
        
        case 0xCC: // CALL Z, a16
        {
            uint16_t addr = cpu_fetch_word();
            if (get_flag(FLAG_Z)) {
                cpu.sp--;
                memory_write(cpu.sp, cpu.pc >> 8);
                cpu.sp--;
                memory_write(cpu.sp, cpu.pc & 0xFF);
                cpu.pc = addr;
            }
            break;
        }
        
        case 0xD0: // RET NC
            if (!get_flag(FLAG_C)) {
                uint8_t low = memory_read(cpu.sp++);
                uint8_t high = memory_read(cpu.sp++);
                cpu.pc = (high << 8) | low;
            }
            break;
            
        case 0xD1: // POP DE
        {
            uint8_t low = memory_read(cpu.sp++);
            uint8_t high = memory_read(cpu.sp++);
            set_de((high << 8) | low);
            break;
        }
        
        case 0xD2: // JP NC, a16
        {
            uint16_t addr = cpu_fetch_word();
            if (!get_flag(FLAG_C)) {
                cpu.pc = addr;
            }
            break;
        }
        
        case 0xD4: // CALL NC, a16
        {
            uint16_t addr = cpu_fetch_word();
            if (!get_flag(FLAG_C)) {
                cpu.sp--;
                memory_write(cpu.sp, cpu.pc >> 8);
                cpu.sp--;
                memory_write(cpu.sp, cpu.pc & 0xFF);
                cpu.pc = addr;
            }
            break;
        }
        
        case 0xD6: // SUB d8
        {
            uint8_t value = cpu_fetch_byte();
            uint8_t result = cpu.a - value;
            
            cpu.f = FLAG_N | (result == 0 ? FLAG_Z : 0) | 
                   ((cpu.a & 0x0F) < (value & 0x0F) ? FLAG_H : 0) |
                   (cpu.a < value ? FLAG_C : 0);
            
            cpu.a = result;
            break;
        }
        
        case 0xD8: // RET C
            if (get_flag(FLAG_C)) {
                uint8_t low = memory_read(cpu.sp++);
                uint8_t high = memory_read(cpu.sp++);
                cpu.pc = (high << 8) | low;
            }
            break;
            
        case 0xDA: // JP C, a16
        {
            uint16_t addr = cpu_fetch_word();
            if (get_flag(FLAG_C)) {
                cpu.pc = addr;
            }
            break;
        }
        
        case 0xDC: // CALL C, a16
        {
            uint16_t addr = cpu_fetch_word();
            if (get_flag(FLAG_C)) {
                cpu.sp--;
                memory_write(cpu.sp, cpu.pc >> 8);
                cpu.sp--;
                memory_write(cpu.sp, cpu.pc & 0xFF);
                cpu.pc = addr;
            }
            break;
        }
        
        case 0xDE: // SBC A, d8
        {
            uint8_t value = cpu_fetch_byte();
            uint8_t carry = get_flag(FLAG_C) ? 1 : 0;
            uint8_t result = cpu.a - value - carry;
            
            cpu.f = FLAG_N | (result == 0 ? FLAG_Z : 0) | 
                   ((cpu.a & 0x0F) < ((value & 0x0F) + carry) ? FLAG_H : 0) |
                   (cpu.a < (value + carry) ? FLAG_C : 0);
            
            cpu.a = result;
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
                // RLC operations (0x00-0x07)
                case 0x07: // RLC A
                    cpu.f = (cpu.a & 0x80) ? FLAG_C : 0;
                    cpu.a = (cpu.a << 1) | ((cpu.f & FLAG_C) ? 0x01 : 0);
                    cpu.f |= (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                    
                // RRC operations (0x08-0x0F)
                case 0x0F: // RRC A
                    cpu.f = (cpu.a & 0x01) ? FLAG_C : 0;
                    cpu.a = (cpu.a >> 1) | ((cpu.f & FLAG_C) ? 0x80 : 0);
                    cpu.f |= (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                    
                // RL operations (0x10-0x17)
                case 0x17: // RL A
                {
                    uint8_t old_carry = get_flag(FLAG_C) ? 1 : 0;
                    cpu.f = (cpu.a & 0x80) ? FLAG_C : 0;
                    cpu.a = (cpu.a << 1) | old_carry;
                    cpu.f |= (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                }
                    
                // RR operations (0x18-0x1F)
                case 0x1F: // RR A
                {
                    uint8_t old_carry = get_flag(FLAG_C) ? 0x80 : 0;
                    cpu.f = (cpu.a & 0x01) ? FLAG_C : 0;
                    cpu.a = (cpu.a >> 1) | old_carry;
                    cpu.f |= (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                }
                    
                // SLA operations (0x20-0x27)
                case 0x27: // SLA A
                    cpu.f = (cpu.a & 0x80) ? FLAG_C : 0;
                    cpu.a <<= 1;
                    cpu.f |= (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                    
                // SRA operations (0x28-0x2F)
                case 0x2F: // SRA A
                    cpu.f = (cpu.a & 0x01) ? FLAG_C : 0;
                    cpu.a = (cpu.a >> 1) | (cpu.a & 0x80);
                    cpu.f |= (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                    
                // SWAP operations (0x30-0x37)
                case 0x37: // SWAP A
                    cpu.a = ((cpu.a & 0x0F) << 4) | ((cpu.a & 0xF0) >> 4);
                    cpu.f = (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                    
                // SRL operations (0x38-0x3F)
                case 0x3F: // SRL A
                    cpu.f = (cpu.a & 0x01) ? FLAG_C : 0;
                    cpu.a >>= 1;
                    cpu.f |= (cpu.a == 0) ? FLAG_Z : 0;
                    break;
                    
                // BIT operations (0x40-0x7F)
                case 0x7C: // BIT 7, H
                    cpu.f = (cpu.f & FLAG_C) | FLAG_H | ((cpu.h & 0x80) ? 0 : FLAG_Z);
                    break;
                    
                case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: // BIT 0
                case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: // BIT 1
                case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: // BIT 2
                case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F: // BIT 3
                case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: // BIT 4
                case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F: // BIT 5
                case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: // BIT 6
                case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7D: case 0x7E: case 0x7F: // BIT 7
                {
                    uint8_t bit = (cb_opcode - 0x40) / 8;
                    uint8_t reg = (cb_opcode - 0x40) % 8;
                    uint8_t value;
                    
                    switch (reg) {
                        case 0: value = cpu.b; break;
                        case 1: value = cpu.c; break;
                        case 2: value = cpu.d; break;
                        case 3: value = cpu.e; break;
                        case 4: value = cpu.h; break;
                        case 5: value = cpu.l; break;
                        case 6: value = memory_read(get_hl()); break;
                        case 7: value = cpu.a; break;
                        default: value = 0; break;
                    }
                    
                    cpu.f = (cpu.f & FLAG_C) | FLAG_H | ((value & (1 << bit)) ? 0 : FLAG_Z);
                    break;
                }
                
                // RES operations (0x80-0xBF)
                case 0x87: // RES 0, A
                    cpu.a &= ~0x01;
                    break;
                    
                // SET operations (0xC0-0xFF)
                case 0xC7: // SET 0, A
                    cpu.a |= 0x01;
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
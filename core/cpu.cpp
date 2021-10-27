#include <iostream>
#include "mmu.hpp"
#include "util.hpp"
#include "cpu.hpp"

#define HALT_EXIT 99

//flags in AF register
const u8 zero_flag_z = 0x80; //it's the 7th bit
const u8 subtraction_flag_n = 0x40; //it's the 6th bit
const u8 half_carry_flag_h = 0x20; //it's the 5th bit
const u8 carry_flag_c = 0x10; //it's the 4th bit

const u8 zero_flag_index = 7;
const u8 subtraction_flag_index = 6;
const u8 half_carry_flag_index = 5;
const u8 carry_flag_index = 4;


CPU::CPU(MMU& mmu){
    this->mmu = mmu;
    
    //setting the pc to start where the boot rom is located
    this->pc = 0;

    //set all register locations and Program counter variables here
    this->af=0x0000;
    this->bc=0x0000;
    this->de=0x0000;
    this->hl=0x0000;
    this->sp=0x0000;
}

u8 CPU::step(){
    //step() should call exec() and return the return value of exec(). 
    //For now, exec() can return 4 (see the comment in CPU.hpp).
    //The reason step() and exec() are separated is because step() will also have to check interrupts and some other things.
    return exec();
}

u8 CPU::exec(){
    //read code from wherever program counter is at
    //increment the program counter so next time we call it we get the next opCode
    //Anytime the program counter is used to read, it needs to be incremented, such as when reading input for an opCode
    u8 opCode = mmu.read(pc++);

    //this is a default value for the cycles used.
    //In each op case, update number of cycles used so the correct number is returned at the end
    u8 cycles = 4;
    

    //There are currently only switch statements for opCodes that a required for booting
    switch(opCode){
        case 0x86: {
            //ADD A,(HL)
            u8 value_at_hl = mmu.read(hl);
            u8 a = getHighByte(af);
            a += value_at_hl;
            setHighByte(&af, a);
            break; 
        }
        case 0xCD: {
            //CALL a16
            sp--;
            sp--;
            mmu.write(sp, pc);

            
            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;
            pc = nn_nn;
            break; 
        }
        case 0xFE: {
            //CP d8
            //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
            u8 n = mmu.read(pc++);
            u8 a = getHighByte(af);
            u8 compare = a - n;

            setSubtractFlag(true);
            setCarryFlag(compare < 0);
            setZeroFlag(compare == 0);

            //TODO: need to look up when to set half cary flag
            break;
        }
        case 0xBE: {
            //CP (HL)
            //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
            u8 valueAtHL = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 compare = a - valueAtHL;

            setSubtractFlag(true);
            setCarryFlag(compare < 0);
            setZeroFlag(compare == 0);

            //TODO: need to look up when to set half cary flag
            break; 
        }
        case 0x3D: {
            //DEC A
            //a--;
            u8 a = getHighByte(af);
            a--;
            setHighByte(&af, a);
            break;
        }
        case 0x05: {
            //DEC B
            //b--;
            u8 b = getHighByte(bc);
            b--;
            setHighByte(&bc, b);
            break;
        }
        case 0x0D: {
            //DEC C
            //c--;
            u8 c = getLowByte(bc);
            c--;
            setLowByte(&bc, c);
            break;
        }
        case 0x15: {
            //DEC D
            //d--;
            u8 d = getHighByte(de);
            d--;
            setHighByte(&de, d);
            break;
        }
        case 0x1D: {
            //DEC E
            //e--;
            u8 e = getLowByte(de);
            e--;
            setLowByte(&de, e);
            break;
        }
        case 0x04: {
            //INC B
            //b++;
            u8 b = getHighByte(bc);
            b++;
            setHighByte(&bc, b);
            break;
        }
        case 0x0C: {
            //INC C
            //c++;
            u8 c = getLowByte(bc);
            c++;
            setLowByte(&bc, c);
            break;
        }
        case 0x13: {
            //INC DE
            de++;
            break;
        }
        case 0x24: {
            //INC H
            //h++;
            u8 h = getHighByte(hl);
            h++;
            setHighByte(&hl, h);
            break;
        }
        case 0x23: {
            //INC HL
            hl++;
            break;
        }
        case 0x20: {
            //JR NZ,r8
            if (readSubtractFlag() && readZeroFlag()){
                //execute relative jump, nn is signed
                s8 nn = mmu.read(pc++);
                pc += nn;
            }
            break;
        }
        case 0x28: {
            //JR Z,r8
            if (readZeroFlag()){
                //execute relative jump, nn is signed
                s8 nn = mmu.read(pc++);
                pc += nn;
            }
            break;
        }
        case 0x18: {
            //JR r8, nn is signed
            s8 nn = mmu.read(pc++);
            pc += nn;
            break;
        }
        case 0x3E: {
            //LD A,d8
            break;
        }
        case 0x1A: {
            //LD A,(DE)
            setHighByte(&af, mmu.read(de));
            break;
        }
        case 0x06: {
            //LD B,d8
            break;
        }
        case 0x0E: {
            //LD C,d8
            break;
        }
        case 0x16: {
            //LD D,d8
            break;
        }
        case 0x11: {
            //LD DE,d16
            break;
        }
        case 0x1E: {
            //LD E,d8
            break;
        }
        case 0x21: {
            //LD HL,d16
            break;
        }
        case 0x2E: {
            //LD L,d8
            break;
        }
        case 0x31: {
            //LD SP,d16
        }
        case 0xE2: {
            //LD (C),A same as LD($FF00+C),A
            mmu.write(getLowByte(bc) + 0xFF00, getHighByte(af));
            // sleep?
            cycles = 8;
            break;
        }
        case 0xEA: {
            //LD (a16),A
            break;
        }
        case 0x32: {
            //LD (HL-),A
            mmu.write(hl--, getHighByte(af));
            // SOMETHING ABOUT A MEMORY SLEEP CYCLE???
            cycles = 8;
            break;
        }
        case 0x22: {
            //LD (HL+),A
            mmu.write(hl++, getHighByte(af));
            // SOMETHING ABOUT A MEMORY SLEEP CYCLE???
            cycles = 8;
            break;
        }
        case 0xF0: {
            //LDH A,(a8) same as LD A,($FF00+a8)
            break;
        }
        case 0xE0: {
            //LDH (a8),A same as LD ($FF00+a8),A

            break;
        }
        case 0xC5: {
            //PUSH BC
            break;
        }
        case 0xC1: {
            //POP BC
            break;
        }
        case 0xC9: {
            //RET
            break;
        }
        case 0x17: {
            //RLA
            break;
        }
        case 0x90: {
            //SUB B
            u8 b = getHighByte(bc);
            u8 a = getHighByte(af);
            u8 res = a - b;
            setHighByte(&af, res);
            break;
        }
        case 0xAF: {
            //XOR A
            u8 a = getHighByte(af);
            a ^= a;
            setHighByte(&af, a);
            break;
        }
        case 0x76: {
            //HALT
            std::cout << "Halt Instruction Reached" << std::endl;
            std::exit(HALT_EXIT);
            return 4;
        }
        case 0x46: case 0x4E: case 0x56: case 0x5E: 
        case 0x66: case 0x6E: case 0x7E: {
            //LD r1, (HL)
            u8 encodedRegister = (opCode - 0x40) / 8;
            u8* r1 = getRegisterFromEncoding(encodedRegister);
            u8 value = mmu.read(hl);
            *r1 = value;
            
            return 8;
        }
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
        case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
        case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
        case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
        case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
        case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F: {
            //LD r1,r2
            u8 *r1 = getRegisterFromEncoding((opCode - 0x40) / 8);
            u8 *r2 = getRegisterFromEncoding(getLowNibble(opCode));
            *r1 = *r2;
            
            return 4;
        }
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77: {
            //LD (HL), r1
            u8 *r1 = getRegisterFromEncoding(getLowNibble(opCode));
            mmu.write(hl, *r1);

            return 8;
        }
        case 0xCB:
            return execCB();
    }

    //return the number of cycles used. 
    return cycles;
}

// Helpers
u8 CPU::execCB() {
    u8 opCode = mmu.read(pc++);
    switch (opCode) {
        case 0x06: {
            //RLC (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0x6);
            u8 value = mmu.read(*hl);
            u8 high_bit = readBit(value, 7);
            value = (value << 1) | high_bit;
            mmu.write(*hl, value);

            setCarryFlag(high_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07: {
            //RLC r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 high_bit = readBit(*r, 7);
            *r = (*r << 1) | high_bit;
            
            setCarryFlag(high_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8;
        }
        case 0x0E: {
            //RRC (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0xE);
            u8 value = mmu.read(*hl);
            u8 low_bit = readBit(value, 0);
            value = (value >> 1) | (low_bit << 7);
            mmu.write(*hl, value);

            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0F: {
            //RRC r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 low_bit = readBit(*r, 0);
            *r = (*r >> 1) | (low_bit << 7);
            
            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8;
        }
        case 0x16: {
            //RL (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0x6);
            u8 value = mmu.read(*hl);
            u8 carry_flag = readCarryFlag();
            u8 high_bit = readBit(value, 7);
            value = (value << 1) | carry_flag;
            mmu.write(*hl, value);

            setCarryFlag(high_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x17: {
            //RL r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 carry_flag = readCarryFlag();
            u8 high_bit = readBit(*r, 7);
            *r = (*r << 1) | carry_flag;
            
            setCarryFlag(high_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8;
        }
        case 0x1E: {
            //RR (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0xE);
            u8 value = mmu.read(*hl);
            u8 carry_flag = readCarryFlag();
            u8 low_bit = readBit(value, 0);
            value = (value >> 1) | (carry_flag << 7);
            mmu.write(*hl, value);

            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1F: {
            //RR r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 carry_flag = readCarryFlag();
            u8 low_bit = readBit(*r, 0);
            *r = (*r >> 1) | (carry_flag << 7);
            
            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8; 
        }
        case 0x26: {
            //SLA (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0x6);
            u8 value = mmu.read(*hl);
            u8 high_bit = readBit(value, 7);
            value = (value << 1);
            mmu.write(*hl, value);

            setCarryFlag(high_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x27: {
            //SLA r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 high_bit = readBit(*r, 7);
            *r = *r << 1;
            
            setCarryFlag(high_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8;
        }
        case 0x2E: {
            //SRA (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0xE);
            u8 value = mmu.read(*hl);
            u8 low_bit = readBit(value, 0);
            u8 high_bit = readBit(value, 7);
            value = (value >> 1) | (high_bit << 7);
            mmu.write(*hl, value);

            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2F: {
            //SRA r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 low_bit = readBit(*r, 0);
            u8 high_bit = readBit(*r, 7);
            *r = (*r >> 1) | (high_bit << 7);
            
            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8;
        }
        case 0x36: {
            //SWAP (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0x6);
            u8 value = mmu.read(*hl);
            value = ((value & 0x0F) << 4 | (value & 0xF0) >> 4);
            mmu.write(*hl, value);

            setCarryFlag(false);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x37: {
            //SWAP r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            *r = ((*r & 0x0F) << 4 | (*r & 0xF0) >> 4);

            setCarryFlag(false);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8;
        }
        case 0x3E: {
            //SRL (HL)
            u16* hl = (u16*) getRegisterFromEncoding(0xE);
            u8 value = mmu.read(*hl);
            u8 low_bit = readBit(value, 0);
            value = value >> 1;
            mmu.write(*hl, value);

            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(value == 0);
            return 16;
        }
        case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3F: {
            //SRL r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 low_bit = readBit(*r, 0);
            *r = *r >> 1;
            
            setCarryFlag(low_bit);
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);
            return 8;
        }
        case 0x46: case 0x4E: case 0x56: case 0x5E: 
        case 0x66: case 0x6E: case 0x76: case 0x7E: {
            //BIT n, (HL)
            u8 index = (opCode - 0x40) / 8;
            u16* hl = (u16*) getRegisterFromEncoding(0x6);
            u8 value = mmu.read(*hl);

            setHalfCarryFlag(true);
            setSubtractFlag(false);
            setZeroFlag(readBit(value, index) == 0);
            return 16;
        }
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
        case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
        case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
        case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
        case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
        case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F: {
            //BIT n, r
            u8 index = (opCode - 0x40) / 8;
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));

            setHalfCarryFlag(true);
            setSubtractFlag(false);
            setZeroFlag(readBit(*r, index) == 0);
            return 8;
        }
        case 0x86: case 0x8E: case 0x96: case 0x9E: 
        case 0xA6: case 0xAE: case 0xB6: case 0xBE: {
            //RES n, (HL)
            u8 index = (opCode - 0x80) / 8;
            u16* hl = (u16*) getRegisterFromEncoding(0x6);
            u8 value = mmu.read(*hl);
            value = clearBit(value, index);
            mmu.write(*hl, value);
            return 16;
        }
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
        case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
        case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7:
        case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7:
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF: {
            //RES n, r
            u8 index = (opCode - 0x80) / 8;
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            *r = clearBit(*r, index);
            return 8;
        }
        case 0xC6: case 0xCE: case 0xD6: case 0xDE: 
        case 0xE6: case 0xEE: case 0xF6: case 0xFE: {
            //SET n, (HL)
            u8 index = (opCode - 0xC0) / 8;
            u16* hl = (u16*) getRegisterFromEncoding(0x6);
            u8 value = mmu.read(*hl);
            value = setBit(value, index);
            mmu.write(*hl, value);
            return 16;
        }
        case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC7:
        case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCF:
        case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD7:
        case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDF:
        case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE7:
        case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEF:
        case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF7:
        case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFF: {
            //SET n, r
            u8 index = (opCode - 0xC0) / 8;
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            *r = setBit(*r, index);
            return 8;
        }
    }
    return -1; // should be unreachable
}



void CPU::setCarryFlag(bool value) {
    af = changeIthBitToX(af, carry_flag_index, value);
}
void CPU::setHalfCarryFlag(bool value) {
    af = changeIthBitToX(af, half_carry_flag_index, value);
}
void CPU::setSubtractFlag(bool value) {
    af = changeIthBitToX(af, subtraction_flag_index, value);
}
void CPU::setZeroFlag(bool value) {
    af = changeIthBitToX(af, zero_flag_index, value);
}

bool CPU::readCarryFlag() {
    return readBit(af, carry_flag_index);
}
bool CPU::readHalfCarryFlag() {
    return readBit(af, half_carry_flag_index);
}
bool CPU::readSubtractFlag() {
    return readBit(af, subtraction_flag_index);
}
bool CPU::readZeroFlag() {
    return readBit(af, zero_flag_index);
}

u8* CPU::getRegisterFromEncoding(u8 nibble) {
    switch (nibble % 8) {
        case 0: // b
            return ((u8*)&bc) + 1;
        case 1: // c
            return ((u8*)&bc);
        case 2: // d
            return ((u8*)&de) + 1;
        case 3: // e
            return ((u8*)&de);
        case 4: // h
            return ((u8*)&hl) + 1;
        case 5: // l
        case 6: // hl
            return ((u8*)&hl);
        case 7: // a
            return ((u8*)&af) + 1;
    }
    return 0; // shoud be unreachable
}
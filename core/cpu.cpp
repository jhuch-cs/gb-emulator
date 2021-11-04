#include <iostream>
#include "mmu.hpp"
#include "util.hpp"
#include "cpu.hpp"

#define HALT_EXIT 99

//flags in AF register
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
        case 0x00: {
            //NOP
            return 4;
        }
        case 0x10: {
            //STOP
            //TODO: Set some interrupt to pause execution until button press?
            return 4;
        }
        case 0x20: {
            //JR NZ, r8 where r8 is signed
            s8 nn = mmu.read(pc++); //always read r8 to consume entire op code
            if (!readZeroFlag()) {
                pc += nn;
                return 12;
            }
            return 8;
        }
        case 0x30: {
            //JR NC, r8 where r8 is signed
            s8 nn = mmu.read(pc++); //always read r8 to consume entire op code
            if (!readCarryFlag()) {
                pc += nn;
                return 12;
            }
            return 8;
        }
        case 0x01: case 0x11: case 0x21: case 0x31: {
            //LD rr,d16
            u16* rr = get16BitRegisterFromEncoding(getHighNibble(opCode));
            u16 n = mmu.read16Bit(pc++);
            pc++; //Incremented twice on 16 bit read
            *rr = n;
            return 12;
        }
        case 0x02: case 0x12: {
            //LD (rr), A
            u16* rr = get16BitRegisterFromEncoding(getHighNibble(opCode));
            u8 a = getHighByte(af);
            mmu.write(*rr, a);
            return 8;
        }
        case 0x22: {
            //LD (HL+), A
            u8 a = getHighByte(af);
            mmu.write(hl++, a);
            return 8;
        }
        case 0x32: {
            //LD (HL-), A
            u8 a = getHighByte(af);
            mmu.write(hl--, a);
            return 8;
        }
        case 0x03: case 0x13: case 0x23: case 0x33: {
            //INC rr
            u16* rr = get16BitRegisterFromEncoding(getHighNibble(opCode));
            *rr += 1;
            return 8;
        }
        case 0x04: case 0x14: case 0x24: {
            //INC r
            u8* r = getRegisterFromEncoding(getHighNibble(opCode) * 2);
            *r += 1;

            setHalfCarryFlag((*r & 0x0F) == 0x00);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);

            return 4;
        }
        case 0x34: {
            //INC (HL)
            u8 value = mmu.read(hl);
            mmu.write(hl, ++value);

            setHalfCarryFlag((value & 0x0F) == 0x00);
            setSubtractFlag(false);
            setZeroFlag(value == 0);

            return 12;
        }
        case 0x05: case 0x15: case 0x25: {
            //DEC r
            u8* r = getRegisterFromEncoding(getHighNibble(opCode) * 2);
            *r -= 1;

            setHalfCarryFlag((*r & 0x0F) == 0x00);
            setSubtractFlag(true);
            setZeroFlag(*r == 0);

            return 4;
        }
        case 0x35: {
            //DEC (HL)
            u8 value = mmu.read(hl);
            mmu.write(hl, --value);

            setHalfCarryFlag((value & 0x0F) == 0x00);
            setSubtractFlag(true);
            setZeroFlag(value == 0);

            return 12;
        }
        case 0x06: case 0x16: case 0x26: {
            //LD r,d8
            u8* r = getRegisterFromEncoding(getHighNibble(opCode) * 2);
            *r = mmu.read(pc++);
            return 8;
        } 
        case 0x36: {
            //LD (HL),d8
            mmu.write(hl, mmu.read(pc++));
            return 12;
        }
        case 0x07: {
            //RLCA
            u8 a = getHighByte(af);
            setHighByte(&af, op_rlc(a));
            return 4;
        }
        case 0x17: {
            //RLA
            u8 a = getHighByte(af);
            setHighByte(&af, op_rl(a));
            return 4;
        }
        case 0x27: {
            //DAA
            // I had to read a blog post just so that I could
            // understand this op code: https://ehaskins.com/2018-01-30%20Z80%20DAA/
            u8 a = getHighByte(af);

            u16 correction = 0;
            if (readHalfCarryFlag() || (!readSubtractFlag() && ((a & 0xf) > 9))) {
                correction |= 0x6;
            }

            if (readCarryFlag() || (!readSubtractFlag && (a > 0x99))) {
                correction |= 0x60;
            }

            if (readSubtractFlag()) {
                a -= correction;
            } else {
                a += correction;
            }
            
            if (((correction << 2) & 0x100) != 0) {
                setCarryFlag(true);
            }

            setZeroFlag(a == 0);
            setHalfCarryFlag(false);

            setHighByte(&af, a);
            return 4;
        }
        case 0x37: {
            //SCF
            setCarryFlag(true);
            setHalfCarryFlag(false);
            setSubtractFlag(false);

            return 4;
        }
        case 0x08: {
            u16 immediate_address = mmu.read16Bit(pc++);
            pc++;
            mmu.write(immediate_address, getLowByte(sp));
            mmu.write(immediate_address + 1, getHighByte(sp));

            return 20;
        }
        case 0x18: {
            //JR r8 where r8 is signed
            s8 nn = mmu.read(pc++);
            pc += nn;

            return 12;
        }
        case 0x28: {
            //JR Z, r8 where r8 is signed
            s8 nn = mmu.read(pc++); //always read r8 to consume entire op code
            if (readZeroFlag()) {
                pc += nn;
                return 12;
            }
            return 8;
        }
        case 0x38: {
            //JR C, r8 where r8 is signed
            s8 nn = mmu.read(pc++); //always read r8 to consume entire op code
            if (readCarryFlag()) {
                pc += nn;
                return 12;
            }
            return 8;
        }
        case 0x09: case 0x19: case 0x29: case 0x39: {
            //ADD HL, rr
            u16* rr = get16BitRegisterFromEncoding(getHighNibble(opCode));
            u32 untruncated_result = hl + *rr;
            u16 result = (u16) untruncated_result;

            //TODO: Iffy on the 16bit half-carry logic here
            setCarryFlag(untruncated_result > 0xFFFF);
            setHalfCarryFlag((hl & 0xFFF) + (*rr & 0xFFF) > 0xFFF);
            setSubtractFlag(false);

            return 8;
        }
        case 0x0A: case 0x1A: {
            //LD A, (rr)
            u16* rr = get16BitRegisterFromEncoding(getHighNibble(opCode));
            u8 value = mmu.read(*rr);
            setHighByte(&af, value);
            return 8;
        }
        case 0x2A: {
            //LD A, (HL+)
            u8 value = mmu.read(hl++);
            setHighByte(&af, value);
            return 8;
        }
        case 0x3A: {
            //LD A, (HL-)
            u8 value = mmu.read(hl--);
            setHighByte(&af, value);
            return 8;
        }
        case 0x0B: case 0x1B: case 0x2B: case 0x3B: {
            //DEC rr
            u16* rr = get16BitRegisterFromEncoding(getHighNibble(opCode));
            *rr -= 1;
            return 8;
        }
        case 0x0C: case 0x1C: case 0x2C: case 0x3C: {
            //INC r
            u8* r = getRegisterFromEncoding(getHighNibble(opCode) * 2 + 1);
            *r += 1;

            setHalfCarryFlag((*r & 0x0F) == 0x00);
            setSubtractFlag(false);
            setZeroFlag(*r == 0);

            return 4;
        }
        case 0x0D: case 0x1D: case 0x2D: case 0x3D: {
            //DEC r
            u8* r = getRegisterFromEncoding(getHighNibble(opCode) * 2 + 1);
            *r -= 1;

            setHalfCarryFlag((*r & 0x0F) == 0x00);
            setSubtractFlag(true);
            setZeroFlag(*r == 0);

            return 4;
        }
        case 0x0E: case 0x1E: case 0x2E: case 0x3E: {
            //LD r, d8
            u8* r = getRegisterFromEncoding(getHighNibble(opCode) * 2 + 1);
            u8 immediate_value = mmu.read(pc++);
            *r = immediate_value;
            return 8;
        }
        case 0x0F: {
            //RRCA
            u8 a = getHighByte(af);
            setHighByte(&af, op_rrc(a));
            return 4;
        }
        case 0x1F: {
            //RRA
            u8 a = getHighByte(af);
            setHighByte(&af, op_rr(a));
            return 4;
        }
        case 0x2F: {
            //CPL (complement)
            u8 a = getHighByte(af);
            setHighByte(&af, ~a);
            setSubtractFlag(true);
            setHalfCarryFlag(true);
            return 4;
        }
        case 0x3F: {
            //CCF
            setCarryFlag(!readCarryFlag());
            setHalfCarryFlag(false);
            setSubtractFlag(false);
            return 4;
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
        case 0xE2: {
            //LD (C),A same as LD($FF00+C),A
            mmu.write(getLowByte(bc) + 0xFF00, getHighByte(af));
            // sleep?
            cycles = 8;
            break;
        }
        case 0x76: {
            //HALT
            //TODO: Suspend until an interrupt occurs
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
        case 0x86: {
            //ADD A,(HL)
            u8 value_at_hl = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 result = op_add(a, value_at_hl);
            setHighByte(&af, result);
            return 8; 
        }
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87: {
            //ADD A,r1
            u8 *r1 = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            u8 result = op_add(a, *r1);
            setHighByte(&af, result);
            return 4;
        }
        case 0x8E: {
            //ADC A, (HL)
            u8 value_at_hl = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 result = op_adc(a, value_at_hl);
            setHighByte(&af, result);
            return 8; 
        }
        case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F: {
            //ADC A, r1
            u8 *r1 = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            u8 result = op_adc(a, *r1);
            setHighByte(&af, result);
            return 4;
        }
        case 0x96: {
            //SUB (HL)
            u8 value = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 result = op_sub(a, value);
            setHighByte(&af, result);
            return 8;
        }
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97: {
            //SUB r
            u8 *r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            u8 result = op_sub(a, *r);
            setHighByte(&af, result);
            return 4;
        }
        case 0x9E: {
            //SBC A,(HL)
            u8 value = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 result = op_sbc(a, value);
            setHighByte(&af, result);
            return 8;
        }
        case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F: {
            //SBC A,r
            u8 *r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            u8 result = op_sbc(a, *r);
            setHighByte(&af, result);
            return 4;
        }
        case 0xA6: {
            //AND (HL)
            u8 value = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 result = op_and(a, value);
            setHighByte(&af, result);
            return 8;
        }
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7: {
            //AND r
            u8 *r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            u8 result = op_and(a, *r);
            setHighByte(&af, result);
            return 4;
        }
        case 0xAE: {
            //XOR (HL)
            u8 value = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 result = op_xor(a, value);
            setHighByte(&af, result);
            return 8;
        }
        case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF: {
            //XOR r
            u8 *r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            u8 result = op_xor(a, *r);
            setHighByte(&af, result);
            return 4;
        }
        case 0xB6: {
            //OR (HL)
            u8 value = mmu.read(hl);
            u8 a = getHighByte(af);
            u8 result = op_or(a, value);
            setHighByte(&af, result);
            return 8;
        }
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7: {
            //OR r
            u8 *r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            u8 result = op_or(a, *r);
            setHighByte(&af, result);
            return 4;
        }
        case 0xBE: {
            //CP (HL)
            u8 valueAtHL = mmu.read(hl);
            u8 a = getHighByte(af);
            op_cp(a, valueAtHL);
            return 8;
        }
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF: {
            //CP r
            u8 *r = getRegisterFromEncoding(getLowNibble(opCode));
            u8 a = getHighByte(af);
            op_cp(a, *r);
            return 4;
        }
        case 0xC0: {
            //RET NZ
            if(readSubtractFlag() & readZeroFlag()){
                pc = mmu.read(sp);
                sp++;
                sp++;
                return 20;
            }
            else{
                return 8;
            }
        }
        case 0xC1: {
            //POP BC
            bc = mmu.read(sp);
            sp++;
            sp++;
            return 12;
        }
        case 0xC2: {
            //JP NZ, a16
            //check if subtraction flag(n) & zero flags are set

            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readZeroFlag() & readSubtractFlag()){
                pc = nn_nn;
                return 16;
            }
            else{
                return 12;
            }
        }
        case 0xC3: {
            //JP a16
            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;
            pc = nn_nn;
            return 16;
        }
        case 0xC4: {
            //CALL NZ, a16
            //check if subtraction flag(n) & zero flags are set

            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readZeroFlag() & readSubtractFlag()){
                sp--;
                sp--;
                mmu.write(sp, pc);

                pc = nn_nn;
                return 24;
            }
            else{
                return 12;
            }
        }
        case 0xC5: {
            //PUSH BC
            sp--;
            sp--;

            mmu.write(sp, bc);
            return 16;
        }
        case 0xC6: {
            //ADD A, d8
            return 4;
        }
        case 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF: {
            //RST 00H, 08H, 10H, 18H, 20H, 28H, 30H, 38H
            sp--;
            sp--;
            mmu.write(sp, pc);

            call_value = (opCode-0xC7);
            pc = call_value;
            return 16;
        }
        case 0xC8: {
            //RET Z
            if(readZeroFlag()){
                pc = mmu.read(sp);
                sp++;
                sp++;
                return 20;
            }
            else{
                return 8;
            }
        }
        case 0xC9: {
            //RET
            return 4;
        }
        case 0xCA: {
            //JP Z, a16
            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readZeroFlag()){
                pc = nn_nn;
                return 16;
            }
            else{
                return 12;
            }
        }
        case 0xCB: {
            //It's a prefix function
            return execCB();
        }
        case 0xCC: {
            //CALL Z, a16
            //check if zero flag is set

            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readZeroFlag()){
                sp--;
                sp--;
                mmu.write(sp, pc);

                pc = nn_nn;
                return 24;
            }
            else{
                return 12;
            }
            return 4;
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
            return 24; 
        }
        case 0xCE: {
            //ADC A, d8
            return 4;
        }
        case 0xD0: {
            //RET NC
            if(readCarryFlag() & readSubtractFlag()){
                pc = mmu.read(sp);
                sp++;
                sp++;
                return 20;
            }
            else{
                return 8;
            }
        }
        case 0xD1: {
            //POP DE
            de = mmu.read(sp);
            sp++;
            sp++;
            return 12;
        }
        case 0xD2: {
            //JP NC, a16
            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readCarryFlag() & readSubtractFlag()){
                pc = nn_nn;
                return 16;
            }
            else{
                return 12;
            }
        }
        case 0xD4: {
            //CALL NC, a16
            //check if subtraction flag(n) & carry flags are set

            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readCarryFlag() & readSubtractFlag()){
                sp--;
                sp--;
                mmu.write(sp, pc);

                pc = nn_nn;
                return 24;
            }
            else{
                return 12;
            }
            return 4;
        }
        case 0xD5: {
            //PUSH DE
            sp--;
            sp--;

            mmu.write(sp, de);
            return 16;
        }
        case 0xD6: {
            //SUB d8
            return 4;
        }
        case 0xD8: {
            //RET C
            if(readCarryFlag()){
                pc = mmu.read(sp);
                sp++;
                sp++;
                return 20;
            }
            else{
                return 8;
            }
        }
        case 0xD9: {
            //RETI
            //return, PC=(SP), SP=SP+2
            pc = mmu.read(sp);
            sp++;
            sp++;

            //enable interrupts (IME=1)
            /************************NOT SURE HOW TO ENABLE INTERRUPTS*********************/
            return 16;
        }
        case 0xDA: {
            //JP C, a16
            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readCarryFlag()){
                pc = nn_nn;
                return 16;
            }
            else{
                return 12;
            }
        }
        case 0xDC: {
            //CALL C, a16
            //check if carry flag is set and jump if so

            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;

            if(readCarryFlag()){
                sp--;
                sp--;
                mmu.write(sp, pc);

                pc = nn_nn;
                return 24;
            }
            else{
                return 12;
            }
        }
        case 0xDE: {
            //SBC A, d8
            //A=A-n-cy
            u8 value = mmu.read(pc++);
            u8 a = getHighByte(af);
            u8 result = op_sbc(a, value);
            setHighByte(&af, result);
            return 8;
        }
        case 0xE0: {
            //LDH (a8), A aka LD ($FF00+a8), A
            u8 input = mmu.read16Bit(pc++);
            pc++;
            mmu.write(input, getHighByte(af));
            return 16;
        }
        case 0xE1: {
            //POP HL
            hl = mmu.read(sp);
            sp++;
            sp++;
            return 12;
        }
        case 0xE5: {
            //PUSH HL
            sp--;
            sp--;

            mmu.write(sp, hl);
            return 16;
        }
        case 0xE6: {
            //AND d8
            u8 n = mmu.read(pc++);
            u8 a = getHighByte(af);
            setHighByte(&af, op_and(a, n));
            return 8;
        }
        case 0xE8: {
            //ADD SP, r8
            //SP = SP +/- dd ; dd is 8-bit signed number
            s8 num = mmu.read(pc++);
            pc = op_add(sp, num));
            return 16;
        }
        case 0xE9: {
            //JP HL
            //jump to HL, PC=HL
            pc = hl;
            return 4;
        }
        case 0xEA: {
            //LD (a16), A
            u16 addressToWrite = mmu.read(pc++);
            pc++;

            mmu.write(addressToWrite, getHighByte(af));
            return 16;
        }
        case 0xEE: {
            //XOR d8
            //A=A xor n
            u8 n = mmu.read(pc++);
            setHighByte(&af, op_or(getHighByte(af), n));
            return 8;
        }
        case 0xF0: {
            //LDH A, (a8) aka LD A, ($FF00+a8)
            u8 input = mmu.read(pc++);
            setHighByte(&af, (mmu.read(0xFF00+input)));
            return 12;
        }
        case 0xF1: {
            //POP AF
            af = mmu.read(sp);
            sp++;
            sp++;
            return 12;
        }
        case 0xF2: {
            //ld A,(FF00+C)
            u8 c = getLowByte(bc);
            setHighByte(&af, (mmu.read(0xFF00+c)));
            return 8;
        }
        case 0xF3: {
            //DI
            //disable interrupts, IME=0
            /*******************NOT SURE HOW TO DO THIS*******************/
            return 4;
        }
        case 0xF5: {
            //PUSH AF
            sp--;
            sp--;

            mmu.write(sp, af);
            return 16;
        }
        case 0xF6: {
            //OR d8
            u8 valueToOr = mmu.read(pc++);

            u8 a = getHighByte(af);
            a = op_or(a, valueToOr);
            setHighByte(&af, a);
            return 8;
        }
        case 0xF8: {
            //LD HL, SP + r8
            u8 valueToAdd = mmu.read(pc++);
            hl = op_add(sp, valueToAdd);
            return 12;
        }
        case 0xF9: {
            //LD SP, HL
            sp = hl;
            return 8;
        }
        case 0xFA: {
            //LD A, (a16)
            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 address = mmu.read16Bit(pc++);
            pc++;

            u8 a = getHighByte(af);
            a = mmu.read(address);
            setHighByte(&af, a);
            return 16;
        }
        case 0xFB: {
            //EI
            //enable interrupts, IME=1
            /*******************NOT SURE HOW TO DO THIS*******************/
            return 4;
        }
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
            u8 value = mmu.read(hl);
            value = op_rlc(value);
            mmu.write(hl, value);
            return 16;
        }
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07: {
            //RLC r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            *r = op_rlc(*r);
            return 8;
        }
        case 0x0E: {
            //RRC (HL)
            u8 value = mmu.read(hl);
            value = op_rrc(value);
            mmu.write(hl, value);
            return 16;
        }
        case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0F: {
            //RRC r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            *r = op_rrc(*r);
            return 8;
        }
        case 0x16: {
            //RL (HL)
            u8 value = mmu.read(hl);
            value = op_rl(value);
            mmu.write(hl, value);
            return 16;
        }
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x17: {
            //RL r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            *r = op_rl(*r);
            return 8;
        }
        case 0x1E: {
            //RR (HL)
            u8 value = mmu.read(hl);
            value = op_rr(value);
            mmu.write(hl, value);
            return 16;
        }
        case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1F: {
            //RR r
            u8* r = getRegisterFromEncoding(getLowNibble(opCode));
            *r = op_rr(*r);
            return 8;
        }
        case 0x26: {
            //SLA (HL)
            u8 value = mmu.read(hl);
            u8 high_bit = readBit(value, 7);
            value = (value << 1);
            mmu.write(hl, value);

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
            u8 value = mmu.read(hl);
            u8 low_bit = readBit(value, 0);
            u8 high_bit = readBit(value, 7);
            value = (value >> 1) | (high_bit << 7);
            mmu.write(hl, value);

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
            u8 value = mmu.read(hl);
            value = ((value & 0x0F) << 4 | (value & 0xF0) >> 4);
            mmu.write(hl, value);

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
            u8 value = mmu.read(hl);
            u8 low_bit = readBit(value, 0);
            value = value >> 1;
            mmu.write(hl, value);

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
            u8 value = mmu.read(hl);

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
            u8 value = mmu.read(hl);
            value = clearBit(value, index);
            mmu.write(hl, value);
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
            u8 value = mmu.read(hl);
            value = setBit(value, index);
            mmu.write(hl, value);
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

u8 CPU::op_add(u8 reg, u8 value) {
    u16 untruncated_result = reg + value;
    u8 result = (u8) untruncated_result;
    
    setCarryFlag(untruncated_result > 0xFF);
    setHalfCarryFlag(((reg & 0xF) + (value & 0xF)) & 0x10);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_adc(u8 reg, u8 value) {
    u16 untruncated_result = reg + value + readCarryFlag();
    u8 result = (u8) untruncated_result;

    setCarryFlag(untruncated_result > 0xFF);
    setHalfCarryFlag(((reg & 0xF) + (value & 0xF) + readCarryFlag()) & 0x10);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_sub(u8 reg, u8 value) {
    u8 result = reg - value;

    setCarryFlag(reg < value); //went negative
    setHalfCarryFlag(((reg & 0xF) - (value & 0xF)) & 0x10);
    setSubtractFlag(true);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_sbc(u8 reg, u8 value) {
    u8 result = reg - value - readCarryFlag();

    setCarryFlag(reg < value + readCarryFlag()); //went negative
    setHalfCarryFlag(((reg & 0xF) - (value & 0xF) - readCarryFlag()) & 0x10);
    setSubtractFlag(true);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_and(u8 reg, u8 value) {
    u8 result = reg & value;

    setCarryFlag(false);
    setHalfCarryFlag(true);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_xor(u8 reg, u8 value) {
    u8 result = reg ^ value;

    setCarryFlag(false);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_or(u8 reg, u8 value) {
    u8 result = reg | value;

    setCarryFlag(false);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
}
void CPU::op_cp(u8 reg, u8 value) {
    setSubtractFlag(true);
    setZeroFlag(reg == value);
    setHalfCarryFlag(((reg & 0xF) - (value & 0xF)) & 0x10);
    setCarryFlag(reg < value);
}

u8 CPU::op_rlc(u8 reg) {
    u8 high_bit = readBit(reg, 7);
    u8 result = (reg << 1) | high_bit;
    
    setCarryFlag(high_bit);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setZeroFlag(result == 0);
    
    return result;
}
u8 CPU::op_rl(u8 reg) {
    u8 carry_flag = readCarryFlag();
    u8 high_bit = readBit(reg, 7);
    u8 result = (reg << 1) | carry_flag;
    
    setCarryFlag(high_bit);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_rrc(u8 reg) {
    u8 low_bit = readBit(reg, 0);
    u8 result = (reg >> 1) | (low_bit << 7);
    
    setCarryFlag(low_bit);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
}
u8 CPU::op_rr(u8 reg) {
    u8 carry_flag = readCarryFlag();
    u8 low_bit = readBit(reg, 0);
    u8 result = (result >> 1) | (carry_flag << 7);
    
    setCarryFlag(low_bit);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setZeroFlag(result == 0);

    return result;
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
    return 0; // should be unreachable
}

u16* CPU::get16BitRegisterFromEncoding(u8 nibble) {
    switch (nibble % 4) {
        case 0: // BC
            return &bc;
        case 1: // DE
            return &de;
        case 2: // HL
            return &hl;
        case 3: // SP
            return &sp;
    }
    return 0; // should be unreachable
}
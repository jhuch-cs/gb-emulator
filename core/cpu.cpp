#pragma once
#include "./mmu.hpp"
#include "./util.hpp"
#include "cpu.hpp"

//flags in AF register
const u8 zero_flag_z = 0x80; //it's the 7th bit
const u8 subtraction_flag_n = 0x40; //it's the 6th bit
const u8 half_carry_flag_h = 0x20; //it's the 5th bit
const u8 carry_flag_c = 0x10; //it's the 4th bit



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
        case 0x86:
            //ADD A,(HL)
            u8 value_at_hl = mmu.read(hl);
            u8 a = getHighByte(af);
            a += value_at_hl;
            setHighByte(&af, a);
            break;

        case 0xCD:
            //CALL a16
            sp--;
            sp--;
            mmu.write(sp, pc);

            
            //PC NEEDS TO BE INCREMENTED TWICE ON 16 BIT READ
            u16 nn_nn = mmu.read16Bit(pc++);
            pc++;
            pc = nn_nn;
            break;

        case 0xFE:
            //CP d8
            //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
            u8 n = mmu.read(pc++);
            u8 compare = a - n;

            //set subtraction flag
            u8 f = getLowByte(af);
            f = f | subtraction_flag_n;
            setHighByte(&af, f);
            
            if (compare < 0){
                //set carry flag
                u8 f = getHighByte(af);
                f = f | carry_flag_c;
                setHighByte(&af, f);
            }
            else if(compare == 0){
                //set zero flag
                u8 f = getHighByte(af);
                f = f | zero_flag_z;
                setHighByte(&af, f);
            }
            //need to look up when to set half cary flag
            break;
        case 0xBE:
            //CP (HL)
            //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
            u8 valueAtHL = mmu.read(hl);
            u8 compare = a - valueAtHL;

            //set subtraction flag
            u8 f = getLowByte(af);
            f = f | subtraction_flag_n;
            setLowByte(&af, f);
            
            if (compare < 0){
                //set carry flag
                u8 f = getHighByte(af);
                f = f | carry_flag_c;
                setLowByte(&af, f);
            }
            else if(compare == 0){
                //set zero flag
                u8 f = getHighByte(af);
                f = f | zero_flag_z;
                setLowByte(&af, f);
            }
            //need to look up when to set half cary flag
            break;

        case 0x3D:
            //DEC A
            //a--;
            u8 a = getHighByte(af);
            a--;
            setHighByte(&af, a);
            break;
        case 0x05:
            //DEC B
            //b--;
            u8 b = getHighByte(bc);
            b--;
            setHighByte(&bc, b);
            break;
        case 0x0D:
            //DEC C
            //c--;
            u8 c = getLowByte(bc);
            c--;
            setLowByte(&bc, c);
            break;
        case 0x15:
            //DEC D
            //d--;
            u8 d = getHighByte(de);
            d--;
            setHighByte(&de, d);
            break;
        case 0x1D:
            //DEC E
            //e--;
            u8 e = getLowByte(de);
            e--;
            setLowByte(&de, e);
            break;

        case 0x04:
            //INC B
            //b++;
            u8 b = getHighByte(bc);
            b++;
            setHighByte(&bc, b);
            break;
        case 0x0C:
            //INC C
            //c++;
            u8 c = getLowByte(bc);
            c++;
            setLowByte(&bc, c);
            break;
        case 0x13:
            //INC DE
            de++;
            break;
        case 0x24:
            //INC H
            //h++;
            u8 h = getHighByte(hl);
            h++;
            setHighByte(&hl, h);
            break;
        case 0x23:
            //INC HL
            hl++;
            break;
        
        case 0x20:
            //JR NZ,r8
            u8 f = getLowByte(af);
            if ((subtraction_flag_n & f) && (zero_flag_z & f)){
                //execute relative jump, nn is signed
                s8 nn = mmu.read(pc++);
                pc += nn;
            }
            break;
        case 0x28:
            //JR Z,r8
            u8 f = getLowByte(af);
            if (zero_flag_z & f){
                //execute relative jump, nn is signed
                s8 nn = mmu.read(pc++);
                pc += nn;
            }
            break;
        case 0x18:
            //JR r8, nn is signed
            s8 nn = mmu.read(pc++);
            pc += nn;
            break;
        
        case 0x3E:
            //LD A,d8
            break;
        case 0x1A:
            //LD A,(DE)
            setHighByte(&af, mmu.read(de));
            break;
        case 0x7C:
            //LD A,H
            u8 h = getHighByte(hl);
            setHighByte(&af, h);
            break;
        case 0x7B:
            //LD A,E
            u8 e = getLowByte(de);
            setHighByte(&af, e);
            break;
        case 0x7D:
            //LD A,L
            u8 l = getLowByte(hl);
            setHighByte(&af, l);
            break;
        case 0x78:
            //LD A,B
            u8 b = getHighByte(bc);
            setHighByte(&af, b);
            break;
        
        case 0x06:
            //LD B,d8
            break;
        
        case 0x0E:
            //LD C,d8
            break;
        case 0x4F:
            //LD C,A
            u8 a = getHighByte(af);
            setLowByte(&bc, a);
            break;
        
        case 0x57:
            //LD D,A
            u8 a = getHighByte(af);
            setHighByte(&de, a);
            break;
        case 0x16:
            //LD D,d8
            break;
        
        case 0x11:
            //LD DE,d16
            break;
        
        case 0x1E:
            //LD E,d8
            break;
        
        case 0x67:
            //LD H,A
            u8 a = getHighByte(af);
            setHighByte(&hl, a);
            break;
        
        case 0x21:
            //LD HL,d16
            break;
        
        case 0x2E:
            //LD L,d8
            break;
        
        case 0x31:
            //LD SP,d16
        
        case 0xE2:
            //LD (C),A same as LD($FF00+C),A
            mmu.write(getLowByte(bc) + 0xFF00, getHighByte(af));
            // sleep?
            cycles = 8;
            break;
        
        case 0xEA:
            //LD (a16),A
            break;
        
        case 0x77:
            //LD (HL),A
            mmu.write(hl, getHighByte(af));
            // SOMETHING ABOUT A MEMORY SLEEP CYCLE???
            cycles = 8;
            break;
        case 0x32:
            //LD (HL-),A
            mmu.write(hl--, getHighByte(af));
            // SOMETHING ABOUT A MEMORY SLEEP CYCLE???
            cycles = 8;
            break;
        case 0x22:
            //LD (HL+),A
            mmu.write(hl++, getHighByte(af));
            // SOMETHING ABOUT A MEMORY SLEEP CYCLE???
            cycles = 8;
            break;

        case 0xF0:
            //LDH A,(a8) same as LD A,($FF00+a8)
            break;
        case 0xE0:
            //LDH (a8),A same as LD ($FF00+a8),A

            break;
        
        case 0xC5:
            //PUSH BC
            break;
        
        case 0xC1:
            //POP BC
            break;
        
        case 0xC9:
            //RET
            break;
        
        case 0x17:
            //RLA
            break;
        
        case 0x90:
            //SUB B
            u8 b = getHighByte(bc);
            u8 a = getHighByte(af);
            u8 res = a - b;
            setHighByte(&af, res);
            break;
        
        case 0xAF:
            //XOR A
            u8 a = getHighByte(af);
            a ^= a;
            setHighByte(&af, a);
            break;
    }

    //return the number of cycles used. 
    return cycles;
    
    //Something called Prefixed ($CB $xx) also had two op codes needed for booting
    //Don't know what the prefixed stuff means, it's found here, it's the second table https://gbdev.io/gb-opCodes/optables/
    //The needed op codes here are for BIT 7,H which is 0xCB7C, and RL C which is 0xCB11
}

// Helpers
u8 CPU::getHighByte(u16 value){
    return ((value>>8) & 0xff);
}
u8 CPU::getLowByte(u16 value){
    return (value & 0xff);
}

u8 CPU::setHighByte(u16* destination, u8 value){
    *destination = (*destination & 0x00ff) | (value << 8);
}
u8 CPU::setLowByte(u16* destination, u8 value){
    *destination = (*destination & 0xff00) | value;
}


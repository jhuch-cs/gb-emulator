#pragma once
#include "./mmu.hpp"
#include "./util.hpp"
#include "cpu.hpp"

//set all register locations and Program counter variables up here I think

//flags in AF register
const u8 zero_flag_z = 0x80; //it's the 7th bit
const u8 subtraction_flag_n = 0x40; //it's the 6th bit
const u8 half_carry_flag_h = 0x20; //it's the 5th bit
const u8 carry_flag_c = 0x10; //it's the 4th bit



CPU::CPU(MMU& mmu, u16* bootRomLocation){
    this->mmu = mmu;
    
    //setting the pc to start where the boot rom is located
    this->pc = *bootRomLocation;
}

u8 CPU::step(){
    //not sure how we should calulcate cycles right now, should be fine if it temporarily just returns 1 I suppose
    return 1;
}

u8 CPU::exec(){
    //while loop continuously reads codes from mmu and executes them
    //idk if the loop just needs to go. I assume there's no reason to end the loop
    while(1){
        //read code from wherever program counter is at
        //increment the program counter so next time we call it we get the next opCode
        //Anytime the program counter is used to read, it needs to be incremented, such as when reading input for an opCode
        u8 opCode = mmu.read(pc++);
        

        //There are currently only switch statements for opCodes that a required for booting
        switch(opCode){
            case 0x86:
                //ADD A,(HL)
                u8 address_hl = mmu.read(hl);
                /***********a += mmu.read(address_hl);**********/
                break;

            case 0xCD:
                //CALL a16
                sp--;
                sp--;
                u16 address_sp = mmu.read16Bit(sp);
                mmu.write(address_sp, pc);

                u16 nn = mmu.read16Bit(pc++);
                pc = nn;
                break;

            case 0xFE:
                //CP d8
                //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
                u8 n = mmu.read(pc++);
                /*********u8 compare = a - n;************/
                //set subtraction flag
                /********if (compare < 0){
                    //set carry flag
                }
                else if(compare == 0){
                    //set zero flag
                }*********/
                //need to look up when to set half cary flag
                break;
            case 0xBE:
                //CP (HL)
                //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
                u8 valueAtHL = mmu.read(hl);
                /**********u8 compare = a - valueAtHL;***********/
                //how/what flags do we set/reset here
                break;

            case 0x3D:
                //DEC A
                //a--;
                break;
            case 0x05:
                //DEC B
                //b--;
                break;
            case 0x0D:
                //DEC C
                //c--;
                break;
            case 0x15:
                //DEC D
                //d--;
                break;
            case 0x1D:
                //DEC E
                //e--;
                break;

            case 0x04:
                //INC B
                //b++;
                break;
            case 0x0C:
                //INC C
                //c++;
                break;
            case 0x13:
                //INC DE
                de++;
                break;
            case 0x24:
                //INC H
                //h++;
                break;
            case 0x23:
                //INC HL
                hl++;
                break;
            
            case 0x20:
                //JR NZ,r8
                /********if ((subtraction_flag_n & f) && (zero_flag_z & f)){
                    //execute jump
                }*********/
                break;
            case 0x28:
                //JR Z,r8
                /*****if (zero_flag_z & f){
                    //execute jump
                }******/
                break;
            case 0x18:
                //JR r8
                break;
            
            case 0x3E:
                //LD A,d8
                break;
            case 0x1A:
                //LD A,(DE)
                break;
            case 0x7C:
                //LD A,H
                break;
            case 0x7B:
                //LD A,E
                break;
            case 0x7D:
                //LD A,L
                break;
            case 0x78:
                //LD A,B
                break;
            
            case 0x06:
                //LD B,d8
                break;
            
            case 0x0E:
                //LD C,d8
                break;
            case 0x4F:
                //LD C,A
                break;
            
            case 0x57:
                //LD D,A
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
                break;
            
            case 0xEA:
                //LD (a16),A
                break;
            
            case 0x77:
                //LD (HL),A
                break;
            case 0x32:
                //LD (HL-),A
                break;
            case 0x22:
                //LD (HL+),A
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
                break;
            
            case 0xAF:
                //XOR A
                break;
        }
    }
    
    //Something called Prefixed ($CB $xx) also had two op codes needed for booting
    //Don't know what the prefixed stuff means, it's found here, it's the second table https://gbdev.io/gb-opCodes/optables/
    //The needed op codes here are for BIT 7,H which is 0x7C, and RL C which is 0x11
}

// Helpers
u8 CPU::getHighByte(u16 value){
    
}
u8 CPU::getLowByte(u16 value){

}

u8 CPU::setHighByte(u16* destination, u8 value){

}
u8 CPU::setLowByte(u16* destination, u8 value){

}

// Map from the register code to the register value itself
u8 CPU::get8BitRegister(u8 registerValue){

}
u16 CPU::get16BitRegister(u8 registerValue){

}


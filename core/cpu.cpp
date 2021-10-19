#pragma once
#include "./mmu.hpp"
#include "./util.hpp"
#include "cpu.hpp"

//set all register locations and Program counter variables up here
//This register setup won't work right now, but it's just so we can easily write the functions for the opcodes
//We'll have to figure out how to structure the registers so we can easily access one or both at the same time
u16 AF;
u16 BC;
u16 DE;
u16 HL;
u16 SP;
u8 A;
u8 B;
u8 C;
u8 D;
u8 E;
u8 H;
u8 L;

u16 PC;
//Not sure what to set all the register locations to, where can we find that?

CPU::CPU(MMU& mmu, u16* bootRomLocation){
    this->mmu = mmu;
    
    //setting the PC to start where the boot rom is located
    this->PC = *bootRomLocation;
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
        //increment the program counter so next time we call it we get the next opcode
        //Anytime the program counter is used to read, it needs to be incremented, such as when reading input for an opcode
        u8 opCode = mmu.read(PC++);
        

        //There are currently only switch statements for opCodes that a required for booting
        switch(opCode){
            case 0x86:
                //ADD A,(HL)
                u8 addressAtHL = mmu.read(HL);
                A += mmu.read(addressAtHL);
                break;

            case 0xCD:
                //CALL a16
                SP--;
                SP--;
                u16 addressAtSP = mmu.read16Bit(SP);
                mmu.write(addressAtSP, PC);

                u16 nn = mmu.read16Bit(PC++);
                PC = nn;
                break;

            case 0xFE:
                //CP d8
                //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
                u8 n = mmu.read(PC++);
                u8 compare = A - n;
                //how/what flags do we set/reset here
                break;
            case 0xBE:
                //CP (HL)
                //CP is a subtraction from A that doesn't update A, only the flags it would have set/reset if it really was subtracted.
                u8 valueAtHL = mmu.read(HL);
                u8 compare = A - valueAtHL;
                //how/what flags do we set/reset here
                break;

            case 0x3D:
                //DEC A
                A--;
                break;
            case 0x05:
                //DEC B
                B--;
                break;
            case 0x0D:
                //DEC C
                C--;
                break;
            case 0x15:
                //DEC D
                D--;
                break;
            case 0x1D:
                //DEC E
                E--;
                break;

            case 0x04:
                //INC B
                B++;
                break;
            case 0x0C:
                //INC C
                C++;
                break;
            case 0x13:
                //INC DE
                DE++;
                break;
            case 0x24:
                //INC H
                H++;
                break;
            case 0x23:
                //INC HL
                HL++;
                break;
            
            case 0x20:
                //JR NZ,r8
                break;
            case 0x28:
                //JR Z,r8
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
    //Don't know what the prefixed stuff means, it's found here, it's the second table https://gbdev.io/gb-opcodes/optables/
    //The needed op codes here are for BIT 7,H which is 0x7C, and RL C which is 0x11
}


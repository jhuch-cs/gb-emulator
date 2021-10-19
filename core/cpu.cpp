#pragma once
#include "./mmu.hpp"
#include "./util.hpp"
#include "cpu.hpp"

//set all register locations and Program counter variables up here
/* uint16_t AF;
  uint16_t BC;
  uint16_t DE;
  uint16_t HL;
  uint16_t SP;*/

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
        u8 opCode = mmu.read(PC++);

        //There are currently only switch statements for opCodes that a required for booting
        switch(opCode){
            case 0x86:
                //ADD A,(HL)
                break;

            case 0xCD:
                //CALL a16
                break;

            case 0xFE:
                //CP d8
                break;
            case 0xBE:
                //CP (HL)
                break;

            case 0x3D:
                //DEC A
                break;
            case 0x05:
                //DEC B
                break;
            case 0x0D:
                //DEC C
                break;
            case 0x15:
                //DEC D
                break;
            case 0x1D:
                //DEC E

            case 0x04:
                //INC B
                break;
            case 0x0C:
                //INC C
                break;
            case 0x13:
                //INC DE
                break;
            case 0x24:
                //INC H
                break;
            case 0x23:
                //INC HL
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


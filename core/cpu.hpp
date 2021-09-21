#pragma once

#include "./mmu.hpp"
#include "./util.hpp"

class CPU {
public: 
  CPU(MMU& mmu);
        
  void step();
  void exec(u8 opcode);

 private:
  MMU mmu;

  // Registers

  // Registers are sometimes combined into 16-bit registers. First register is the high-byte.
  // `f` is the flag register for zero, overflow, negative flags
  u16 af, bc, de, hl;

  // Special Registers
  // `sp` is stack pointer, `pc` is program counter
  u16 sp, pc;


  // Helpers
  u8 getHighByte(u16 value);
  u8 getLowByte(u16 value);

  u8 setHighByte(u16* destination, u8 value);
  u8 setLowByte(u16* destination, u8 value);

  // Map from the register code to the register value itself
  u8 get8BitRegister(u8 registerValue);
  u16 get16BitRegister(u8 registerValue);


  // TODO: Like 200 op-codes and stack management functions, too
};


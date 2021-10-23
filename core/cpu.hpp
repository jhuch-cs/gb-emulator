#pragma once

#include "./mmu.hpp"
#include "./util.hpp"

class CPU {
public: 
  // At construction time, `exec` the boot rom
  CPU(MMU& mmu, u8* bootRomLocation);
  
  // Ultimately, `step()` should return the number of cycles required to completely execute the op code
  // that we processed this step. But timing is not mission critical at the moment, so you can just 
  // hard-code 4 as the return value for now. 
  // When the time comes, https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html is a great quick reference,
  // and it lists the number of cycles required. Note that some resources use 'machine cycles' which is using the 
  // CPU's 4Mhz clock as a reference and some use cycles based on the MMU's 1MHz clock. For our purposes, 
  // we're using cycles based the MMU's lower clock (which is what the above link uses), so some instructions take as many
  // as 24 cycles to complete
  u8 step();

  // Rely on the `pc` for the exec location
  u8 exec();

 private:
  MMU mmu;

  // Registers

  // Registers are sometimes combined into 16-bit registers. First register is the high-byte.
  // `f` is the flag register for zero, subtraction, half-carry, full-carry flags
  // https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
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
  // Op codes: pg. 65, http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
};


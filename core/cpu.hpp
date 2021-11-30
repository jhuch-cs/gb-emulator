#pragma once

#include "./mmu.hpp"
#include "./util.hpp"
#include "./sharedmemorystructs.hpp"

enum Interrupt {
  VBLANK_INT   = 0, //avoid conflict with Mode::VBLANK
  LCD_STAT = 1,
  TIMER    = 2,
  SERIAL   = 3,
  INPUT    = 4, //ie JOYPAD
  NONE     = -1,
};

class CPU {
public: 
  // At construction time, `exec` the boot rom
  CPU(MMU* mmu, CPU_Shared_Mem* CPU_Shared);
  
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

  u8 handleInterrupts();
  void requestInterrupt(Interrupt interrupt);
  void acknowledgeInterrupt(Interrupt interrupt);
 private:
  MMU* mmu;
  CPU_Shared_Mem* CPU_Shared;
  // Registers

  // Registers are sometimes combined into 16-bit registers. First register is the high-byte.
  // `f` is the flag register for zero, subtraction, half-carry, full-carry flags
  // https://gbdev.io/pandocs/CPU_Registers_and_Flags.html
  u16 af, bc, de, hl;

  // Special Registers
  // `sp` is stack pointer, `pc` is program counter
  u16 sp, pc;

  // IME is the Interrupt Master Enable flag
  bool ime = false;
  bool halted = false;
  Interrupt checkInterrupts();
  u16 getInterruptVector(Interrupt interrupt);

  bool logMode = false;

  u8 execCB();

  void setCarryFlag(bool value);
  void setHalfCarryFlag(bool value);
  void setSubtractFlag(bool value);
  void setZeroFlag(bool value);

  u8 op_add(u8 reg, u8 value);
  u8 op_adc(u8 reg, u8 value);
  u8 op_sub(u8 reg, u8 value);
  u8 op_sbc(u8 reg, u8 value);
  u8 op_and(u8 reg, u8 value);
  u8 op_xor(u8 reg, u8 value);
  u8 op_or(u8 reg, u8 value);
  void op_cp(u8 reg, u8 value);

  u8 op_rlc(u8 reg);
  u8 op_rl(u8 reg);
  u8 op_rrc(u8 reg);
  u8 op_rr(u8 reg);

  void pushToStack(u16 value);
  u16 popFromStack();
  
  bool readCarryFlag();
  bool readHalfCarryFlag();
  bool readSubtractFlag();
  bool readZeroFlag();

  u8* getRegisterFromEncoding(u8 nibble);
  u16* get16BitRegisterFromEncoding(u8 nibble);

  // TODO: Like 200 op-codes and stack management functions, too
  // Op codes: pg. 65, http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
};


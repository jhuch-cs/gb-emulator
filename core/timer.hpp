#pragma once

#include "./mmu.hpp"
#include "./cpu.hpp"
#include "./util.hpp"

// The value in bits 0-1 of 0xFF07 control the timer speed 
// CPU_Clock / TIMER_DIVISOR
enum TIMER_DIVISOR {
  d1024 = 0b00,
  d16   = 0b01,
  d64   = 0b10,
  d256  = 0b11,
};

class Timer {
public:
  Timer(MMU& mmu, CPU& cpu);

  void step(u8 cpuCyclesElapsed);

  void resetDiv();
private:
  MMU mmu;
  CPU cpu;

  u16 divCyclesLeft  = 0;
  u16 timaCyclesLeft = 0;
  
  bool timerEnabled();
  u16 getDivisor();
};
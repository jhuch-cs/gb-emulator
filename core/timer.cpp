#include "./mmu.hpp"
#include "./timer.hpp"
#include "./util.hpp"

Timer::Timer(MMU& mmu, CPU& cpu) : mmu(mmu), cpu(cpu) {}

// cpuCyclesElapsed is measured by memory cycles
void Timer::step(u8 cpuCyclesElapsed) {
  divCyclesLeft += cpuCyclesElapsed;

  // DIV is always counting at 16384Hz (CPU_Clock / 256)
  while (divCyclesLeft >= 256) {
    u8 divTimer = mmu.read(DIV_ADDRESS);
    divTimer++;
    //write directly to avoid access rules around DIV_ADDRESS
    mmu.writeDirectly(DIV_ADDRESS, divTimer);
    divCyclesLeft -= 256;
  }

  // TIMA counts conditionally and variably based on 0xFF07
  if (timerEnabled()) {
    timaCyclesLeft += cpuCyclesElapsed;
    u16 divisor = getDivisor();
    while (timaCyclesLeft >= divisor) {
      u8 timerCounter = mmu.read(TIMA_ADDRESS);
      if (timerCounter == 0xFF) { // Will overflow
        timerCounter = mmu.read(TMA_ADDRESS);
        cpu.requestInterrupt(TIMER);
      } else {
        timerCounter++;
      }
      mmu.write(TIMA_ADDRESS, timerCounter);
      timaCyclesLeft -= divisor;
    }
  }
}

bool Timer::timerEnabled() {
  return readBit(mmu.read(TAC_ADDRESS), 2);
}

u16 Timer::getDivisor() {
  // interpret bottom two bits as enum
  switch (static_cast<TIMER_DIVISOR>(mmu.read(TAC_ADDRESS) & 0b11)) {
    case d1024: 
      return 1024;
    case d16:
      return 16;
    case d64: 
      return 64;
    case d256: 
      return 256;
    default:
      return 256;
  }
}
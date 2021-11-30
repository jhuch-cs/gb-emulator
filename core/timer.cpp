#include "./timer.hpp"

Timer::Timer(CPU* cpu, Timer_Shared_Mem* Timer_Shared) 
  : cpu(cpu), Timer_Shared(Timer_Shared) {}

// cpuCyclesElapsed is measured by memory cycles
void Timer::step(u8 cpuCyclesElapsed) {
  divCyclesLeft += cpuCyclesElapsed;

  // DIV is always counting at 16384Hz (CPU_Clock / 256)
  while (divCyclesLeft >= 256) {
    Timer_Shared->div++;
    divCyclesLeft -= 256;
  }

  // TIMA counts conditionally and variably based on 0xFF07
  if (timerEnabled()) {
    timaCyclesLeft += cpuCyclesElapsed;
    u16 divisor = getDivisor();
    while (timaCyclesLeft >= divisor) {
      if (Timer_Shared->tima == 0xFF) { // Will overflow
        Timer_Shared->tima = Timer_Shared->tma;
        cpu->requestInterrupt(TIMER);
      } else {
        Timer_Shared->tima++;
      }
      timaCyclesLeft -= divisor;
    }
  }
}

bool Timer::timerEnabled() {
  return readBit(Timer_Shared->tac, 2);
}

u16 Timer::getDivisor() {
  // interpret bottom two bits as enum
  switch (static_cast<TIMER_DIVISOR>(Timer_Shared->tac & 0b11)) {
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
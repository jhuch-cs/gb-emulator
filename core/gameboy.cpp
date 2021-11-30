#include "./gameboy.hpp"
#include "./sharedmemorystructs.hpp"

const int CYCLES_PER_STEP = 69905;


GameBoy::GameBoy(u8* boot_rom, Cartridge* cartridge) : 
  cartridge(cartridge),
  input(new Input())
  {
    PPU_Shared_Mem* PPU_Shared = new PPU_Shared_Mem();
    CPU_Shared_Mem* CPU_Shared = new CPU_Shared_Mem();
    Timer_Shared_Mem* Timer_Shared = new Timer_Shared_Mem();
    mmu = new MMU(cartridge, input, boot_rom, PPU_Shared, CPU_Shared, Timer_Shared);
    cpu = new CPU(mmu, CPU_Shared);
    timer = new Timer(cpu, Timer_Shared);
    ppu = new PPU(PPU_Shared, cpu);
  }


void GameBoy::step() {
  int cyclesThisStep = 0;

  while (cyclesThisStep < CYCLES_PER_STEP) {
    int cycles = cpu->step();
    for (int i = 0; i < 10; i++) { cycles += cpu->step(); }
    cyclesThisStep += cycles;
    timer->step(cycles);
    ppu->step(cycles);
  }
}

u8* GameBoy::getFrameBuffer() {
  return ppu->getFrameBuffer();
}

const char* GameBoy::getTitle() {
  return cartridge->getTitle();
}

void GameBoy::pressButton(Button button) {
  input->pressButton(button);
}
void GameBoy::unpressButton(Button button) {
  input->unpressButton(button);
}
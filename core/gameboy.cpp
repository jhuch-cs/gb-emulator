#include "./gameboy.hpp"
#include <stdio.h>

const int CYCLES_PER_STEP = 69905;


GameBoy::GameBoy(u8* boot_rom, int bootRomSize, Cartridge* cartridge) : 
  cartridge(cartridge),
  input(new Input()) {
    bool runAsCGB = (bootRomSize > 0x100);

    mmu = new MMU(cartridge, input, boot_rom, runAsCGB);
    cpu = new CPU(mmu);
    timer = new Timer(mmu, cpu);
    ppu = new PPU(mmu, cpu, runAsCGB);
  }


void GameBoy::step() {
  int cyclesThisStep = 0;

  while (cyclesThisStep < CYCLES_PER_STEP) {
    int cycles = cpu->step();
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
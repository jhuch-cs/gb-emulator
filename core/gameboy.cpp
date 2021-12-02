#include "./gameboy.hpp"
#include <stdio.h>

const int CYCLES_PER_STEP = 69905;


GameBoy::GameBoy(u8* boot_rom, Cartridge* cartridge) : 
  cartridge(cartridge),
  input(new Input()) {
    if (cartridge->supportsCGB()) {
      printf("Attempting to run as GameBoy Color\n");
    }

    mmu = new MMU(cartridge, input, boot_rom, cartridge->supportsCGB());
    cpu = new CPU(mmu);
    timer = new Timer(mmu, cpu);
    ppu = new PPU(mmu, cpu, cartridge->supportsCGB());
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
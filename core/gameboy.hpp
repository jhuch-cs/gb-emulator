#pragma once

#include "./util.hpp"
#include "./cartridge.hpp"
#include "./input.hpp"
#include "./mmu.hpp"
#include "./cpu.hpp"
#include "./timer.hpp"
#include "./ppu.hpp"

class GameBoy {
public:
  GameBoy(u8* boot_rom, Cartridge* cartridge);

  void step();

  u8* getFrameBuffer();
  const char* getTitle(); 

  void pressButton(Button button);
  void unpressButton(Button button);

  void swapPalettes();
private:
  Cartridge* cartridge;
  Input* input;
	MMU* mmu;
	CPU* cpu;
	Timer* timer;
	PPU* ppu;
  PaletteSwapper* paletteSwapper;
};
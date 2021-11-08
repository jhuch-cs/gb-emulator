#pragma once

#include "./util.hpp"

const u16 BOOT_ROM_SIZE = 0x100;
const u16 BANK_SIZE = 32768; //32 kB

class Cartridge {
public:
  Cartridge(u32 romSize);
  ~Cartridge();

  // public to facilitate rapid writes to mmu memory
  u8* gameRom;
  u8* bootRom;
  u32 gameRomSize;
};
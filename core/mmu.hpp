#pragma once

#include "./util.hpp"
#include "./cartridge.hpp"
#include "./input.hpp"

const u16 CGB_BOOT_ROM_SIZE = 0x8FF;//TODO: Use correct bios file for MMU
const u16 BOOT_ROM_SIZE = 0x100;

const u16 CARTRIDGE_END = 0x7FFF;
const u16 VRAM_START = 0x8000;
const u16 VRAM_END = 0x9FFF;
const u16 CARTRIDGE_RAM_END = 0xBFFF;
const u16 WRAM_BANK0_END = 0xCFFF;
const u16 WRAM_SWITCH_END = 0xDFFF;

const u16 INPUT_ADDRESS = 0xFF00;
const u16 DIV_ADDRESS = 0xFF04;
const u16 TIMA_ADDRESS = 0xFF05;
const u16 TMA_ADDRESS = 0xFF06;
const u16 TAC_ADDRESS = 0xFF07;
const u16 DISABLE_BOOT_ROM = 0xFF50;
const u16 IF_ADDRESS = 0xFF0F;
const u16 IE_ADDRESS = 0xFFFF;
const u16 SB_ADDRESS = 0xFF01;
const u16 SC_ADDRESS = 0xFF02;
const u16 STAT_ADDRESS = 0xFF41;
const u16 DMA_TRSFR_ADDRESS = 0xFF46;
const u16 KEY1 = 0xFF4D; //Speed Switch
const u16 VRAM_BANK = 0xFF4F;
const u16 HDMA1_SOURCE = 0xFF51; //HIGH
const u16 HDMA2_SOURCE = 0xFF52; //LOW
const u16 HDMA3_DEST = 0xFF53; //HIGH
const u16 HDMA4_DEST = 0xFF54; //LOW
const u16 HDMA5_OPTS = 0xFF55; //lower 7 = address, upper 1 = Mode
const u16 OBJECT_PRIORITY = 0xFF6C; //bit 0: 0=OAM, 1=X-coord
const u16 ADDRESS_BG_CRAM = 0xFF68;
const u16 VALUE_BG_CRAM = 0xFF69;
const u16 ADDRESS_OBJ_CRAM = 0xFF6A;
const u16 VALUE_OBJ_CRAM = 0xFF6B;
const u16 WRAM_BANK = 0xFF70; //bottom 3 bits select bank, never 0

class MMU {
public: 
  MMU(Cartridge* cartridge, Input* input, u8* bootRom, bool supportsCGB);
  ~MMU();

  u8 read(u16 address, bool isPPU = false);
  u16 read16Bit(u16 address, bool isPPU = false);

  void write(u16 address, u8 value, bool isPPU = false);
  void writeDirectly(u16 address, u8 value);
  u8 readDirectly(u16 address);

  bool blockedByPPU(u16 address);

  // Public to allow PPU access to both banks directly
  u8(*vram)[0x2000] = new u8[2][0x2000]; //contiguous 2d array
  bool vramBank = 0;

  // Public to allow PPU access
  u8 bg_cram[0x40];
  u8 obj_cram[0x40];
private:
  Cartridge* cartridge;
  Input* input; 
  // Array of size 0x10000 (Addresses 0x0 - 0xFFFF)
  // Some access rules: https://gbdev.io/pandocs/Memory_Map.html
  u8 memory[0x10000] = {}; 

  u8(*wram)[0x1000] = new u8[8][0x1000]; //contiguous 2d array
  u8 wramBank = 1;

  u8* bootRom;
  bool bootRomDisabled = false;

  bool supportsCGB;

  void doGDMA();
  void doHDMA();
};
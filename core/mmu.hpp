#pragma once

#include "./util.hpp"
#include "./cartridge.hpp"

const u16 INPUT_ADDRESS = 0xFF00;
const u16 DIV_ADDRESS = 0xFF04;
const u16 TIMA_ADDRESS = 0xFF05;
const u16 TMA_ADDRESS = 0xFF06;
const u16 TAC_ADDRESS = 0xFF07;
const u16 ENABLE_BOOT_ROM = 0xFF50;
const u16 IF_ADDRESS = 0xFF0F;
const u16 IE_ADDRESS = 0xFFFF;
const u16 SB_ADDRESS = 0xFF01;
const u16 SC_ADDRESS = 0xFF02;

enum Button {
  RIGHT = 0,
  A = 1,
  LEFT = 2,
  B = 3,
  UP = 4,
  SELECT = 5,
  DOWN = 6,
  START = 7,
};

class MMU {
public: 
  MMU(Cartridge& cartridge);
  ~MMU();

  u8 read(u16 address);
  u16 read16Bit(u16 address);

  void write(u16 address, u8 value);

  void pressButton(Button button);
  void unpressButton(Button button);
private:
  Cartridge cartridge;
  // Array of size 0x10000 (Addresses 0x0 - 0xFFFF)
  // Some access rules: https://gbdev.io/pandocs/Memory_Map.html
  u8* memory; 

  u8 readInput();
  void writeInput(u8 byte);
};
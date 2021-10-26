#pragma once 

#include "./util.hpp"

class MMU {
public: 
  MMU();
  ~MMU();

  u8 read(u16 address);
  u16 read16Bit(u16 address);

  void write(u16 address, u8 value);

private:
  // Array of size 0x10000 (Addresses 0x0 - 0xFFFF)
  // Some access rules: https://gbdev.io/pandocs/Memory_Map.html
  u8* memory; 

};
#pragma once

#include <string>
#include "./util.hpp"

const u16 BOOT_ROM_SIZE = 0x100;
const u16 TITLE_ADDRESS = 0x134;
const u16 MBC_TYPE_ADDRESS = 0x147;
const u16 ROM_SIZE_ADDRESS = 0x148;
const u16 RAM_SIZE_ADDRESS = 0x149;
const u16 ROM_BANK_SIZE = 32768; //32 kB
const u16 RAM_BANK_SIZE = 8192; //8 bB

enum MBCType {
  NO_MBC, //Tetris and Dr. Mario have no MBC at all
  MBC_1,
  MBC_2,
  MBC_3,
  MBC_5,
  OTHER,
};

MBCType getMBCType(u8 code);
u32 getRomSize(u8 code);
u32 getRamSize(u8 code);

class CartridgeInfo { //lazy-man's struct
public:
  std::string title;

  MBCType type;
  u32 romSize;
  u32 ramSize;
};

CartridgeInfo getInfo(u8* rom);

class Cartridge {
public:
  Cartridge(u8* rom, CartridgeInfo cartridgeInfo);
  virtual ~Cartridge();

  virtual u8 read(u16 address);
  virtual void write(u16 address, u8 value);
protected:
  u8* rom;
  u8* ram;

  CartridgeInfo cartridgeInfo;
};

Cartridge* createCartridge(u8* rom);



class NoMBC: public Cartridge {
public:
  NoMBC(u8* rom, CartridgeInfo cartridgeInfo);

  u8 read(u16 address) override;
  void write(u16 address, u8 value) override;
};



class MBC1: public Cartridge {
public:
  MBC1(u8* rom, CartridgeInfo cartridgeInfo);

  u8 read(u16 address) override;
  void write(u16 address, u8 value) override;
private:
  u8 romBank = 0x01;
  u8 ramBank = 0x00;
  bool ramEnabled = false;
  bool romBankingMode = true;
};



class MBC3: public Cartridge {
public:
  MBC3(u8* rom, CartridgeInfo cartridgeInfo);

  u8 read(u16 address) override;
  void write(u16 address, u8 value) override;
private:
  u8 romBank = 0x01;
  u8 ramBank = 0x00;
  bool ramEnabled = false;
  bool ramOverRTC = true;
  u8 mappedRegister = 0x00;
  bool romBankingMode = true;
};
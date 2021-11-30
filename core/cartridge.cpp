#include "./cartridge.hpp"
#include <stdio.h>

MBCType getMBCType(u8 code) {
  switch (code) {
    case 0x00:
      return NO_MBC;
    case 0x01: case 0x02: case 0x03:
      return MBC_1;
    case 0x05: case 0x06:
      return MBC_2;
    case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
      return MBC_3;
    case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
      return MBC_5;
    default:
      return OTHER;
  }
}
u32 getRomSize(u8 code) {
  return ROM_BANK_SIZE << code;
}
u32 getRamSize(u8 code) {
  switch (code) {
    case 0x00: case 0x01: 
      return 0;
    case 0x02:
      return RAM_BANK_SIZE;
    case 0x03:
      return 4 * RAM_BANK_SIZE;
    case 0x04:
      return 16 * RAM_BANK_SIZE;
    case 0x05: 
      return 8 * RAM_BANK_SIZE;
    default:
      return 0;
  }
}
CartridgeInfo getInfo(u8* rom) {
  CartridgeInfo info = CartridgeInfo();
  info.title = std::string((char*)(rom + TITLE_ADDRESS));
  info.type = getMBCType(rom[MBC_TYPE_ADDRESS]);
  info.romSize = getRomSize(rom[ROM_SIZE_ADDRESS]);
  info.ramSize = getRamSize(rom[RAM_SIZE_ADDRESS]);
  return info;
}

Cartridge::Cartridge(u8* rom, CartridgeInfo cartridgeInfo) : rom(rom), cartridgeInfo(cartridgeInfo) {
  ram = cartridgeInfo.ramSize ? new u8[cartridgeInfo.ramSize] : NULL;
}
Cartridge::~Cartridge() {}
u8 Cartridge::read(u16 address) {
  return rom[address];
}
void Cartridge::write(u16 address, u8 value) {
  //Do nothing
}
const char* Cartridge::getTitle() {
  return cartridgeInfo.title.c_str();
}



NoMBC::NoMBC(u8* rom, CartridgeInfo cartridgeInfo) : Cartridge(rom, cartridgeInfo)  {}
u8 NoMBC::read(u16 address) {
  return rom[address];
}
void NoMBC::write(u16 address, u8 value) {
  //Do nothing
}



MBC1::MBC1(u8* rom, CartridgeInfo cartridgeInfo) : Cartridge(rom, cartridgeInfo) {}
u8 MBC1::read(u16 address) {
  if (address <= 0x3FFF) {
    return rom[address];
  } else if (address <= 0x7FFF) {
    u32 start_of_rom_bank = 0x4000 * romBank;
    u16 address_requested = address - 0x4000; //0x0000-0x3FFF
    return rom[start_of_rom_bank + address_requested];
  } else if (0xA000 <= address && address <= 0xBFFF) {
    if (!ramEnabled) {
      return 0x00;
    } else {
      u32 start_of_ram_bank = 0x2000 * ramBank;
      u16 address_requested = address - 0xA000; //0x0000-0x1FFF
      return ram[start_of_ram_bank + address_requested];
    }
  }
  printf("ERROR :: Attempted cartridge read from illegal address\n");
  return 0x00;
}
void MBC1::write(u16 address, u8 value) {
  if (address <= 0x1FFF) {
    if (value == 0x00) {
      ramEnabled = false;
    } else if (getLowNibble(value) == 0xA) {
      ramEnabled = true;
    }
  } else if (address <= 0x3FFF) {
    value &= 0x1F; //discard top 3 bits
    if (value == 0x00 || value == 0x20 || value == 0x40 || value == 0x60) {
      romBank = value + 1;
    } else {
      romBank = value;
    }
  } else if (address <= 0x5FFF) {
    //TODO: Select Ram bank or upper bits of ROM bank number?
  } else if (address <= 0x7FFF) {
    romBankingMode = value;
  } else if (0xA000 <= address && address <= 0xBFFF) {
    if (!ramEnabled) { return; }

    u32 start_of_ram_bank = 0x2000 * ramBank;
    u16 address_requested = address - 0xA000; //0x0000-0x1FFF
    ram[start_of_ram_bank + address_requested] = value;
  } else {
    printf("ERROR :: Attempted cartridge write to illegal address\n");
  }
}



MBC3::MBC3(u8* rom, CartridgeInfo cartridgeInfo) : Cartridge(rom, cartridgeInfo) {}
u8 MBC3::read(u16 address) {
  if (address <= 0x3FFF) {
    return rom[address];
  } else if (address <= 0x7FFF) {
    u32 start_of_rom_bank = 0x4000 * romBank;
    u16 address_requested = address - 0x4000; //0x0000-0x3FFF
    return rom[start_of_rom_bank + address_requested];
  } else if (0xA000 <= address && address <= 0xBFFF) {
    if (0x00 <= mappedRegister && mappedRegister <= 0x07) {
      u32 start_of_ram_bank = 0x2000 * ramBank;
      u16 address_requested = address - 0xA000; //0x0000-0x1FFF
      return ram[start_of_ram_bank + address_requested];
    }
  } else {
    printf("ERROR :: Attempted cartridge read from illegal address\n");
    return 0xFF;
  }
  return 0xFF;
}
void MBC3::write(u16 address, u8 value) {
  if (address <= 0x1FFF) {
    ramEnabled = value == 0x0A;
    ramOverRTC = !(value == 0x0A);
  } else if (address <= 0x3FFF) {
    u8 bank = value & 0x7F;
    romBank &= 0x80;
    romBank |= bank;
    if (romBank == 0) { romBank++; }
  } else if (address <= 0x5FFF) {
    mappedRegister = value;
    if (ramEnabled) {
      ramBank = value & 0x3;
    } else {
      u8 bank = (value & 0x3) << 5;
      romBank |= bank;
    }
  } else if (address <= 0x7FFF) {
    //TODO: Implement Clock latch
    // https://gbdev.io/pandocs/MBC3.html#6000-7fff---latch-clock-data-write-only
  } else if (0xA000 <= address && address <= 0xBFFF) {
    if (0x00 <= mappedRegister && mappedRegister <= 0x07) {
      if (!ramEnabled) { return; }

      u32 start_of_ram_bank = 0x2000 * ramBank;
      u16 address_requested = address - 0xA000; //0x0000-0x1FFF
      ram[start_of_ram_bank + address_requested] = value;
    }
    //TODO: Some RTC stuff
  }
}




Cartridge* createCartridge(u8* rom) {
  CartridgeInfo cartridgeInfo = getInfo(rom);
  switch (cartridgeInfo.type) {
    case NO_MBC:
      return new NoMBC(rom, cartridgeInfo);
    case MBC_1: 
      return new MBC1(rom, cartridgeInfo);
    case MBC_3:
      return new MBC3(rom, cartridgeInfo);
    default:
      printf("ERROR :: Cartridge type not currently supported\nERROR :: Program will probably crash\n");
      return new Cartridge(rom, cartridgeInfo);
  }
}
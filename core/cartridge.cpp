#include "./cartridge.hpp"

Cartridge::Cartridge(u32 gameRomSize) : gameRomSize(gameRomSize) {
    gameRom = new u8[gameRomSize];
    bootRom = new u8[BOOT_ROM_SIZE];
}

Cartridge::~Cartridge() {
    delete[] gameRom;
    delete[] bootRom;
}
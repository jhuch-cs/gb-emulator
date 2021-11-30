#include <iostream>
#include <cstring>
#include <stdio.h>
#include "./mmu.hpp"

MMU::MMU(Cartridge* cartridge, Input* input, u8* bootRom) : cartridge(cartridge), input(input), bootRom(bootRom) {
    memory[INPUT_ADDRESS] = 0xFF; // Input starts high, since high = unpressed
    memory[DIV_ADDRESS] = 0x00;
    memory[TIMA_ADDRESS] = 0x00;
}

MMU::~MMU() {}

// During mode OAM: CPU cannot access OAM
// During mode VRAM: CPU cannot access VRAM or OAM
// During restricted modes, any attempt to read returns $FF, any attempt to write are ignored
bool MMU::blockedByPPU(u16 address) {
    u8 stat = memory[STAT_ADDRESS];
    stat &= 0x3;
    // OAM
    if (stat == 2) {
        return (address >= 0xFE00 && address <= 0xFE9F);
    }
    // VRAM
    if (stat == 3) {
        return (address >= 0xFE00 && address <= 0xFE9F) || (address >= 0x8000 && address <= 0x97FF);
    }
    return false;
}

u8 MMU::read(u16 address) {
    if (blockedByPPU(address)) {
        return 0xFF;
    }
    if (address <= 0x7FFF) { //cartridge rom
        if (address < BOOT_ROM_SIZE && !bootRomDisabled) {
            return bootRom[address];
        }
        return cartridge->read(address);
    } else if (0xA000 <= address && address <= 0xBFFF) { //cartridge ram
        return cartridge->read(address);
    } else if (address == INPUT_ADDRESS) {
        return input->readInput();
    }
    return memory[address];
}

u16 MMU::read16Bit(u16 address) {
    return (u16(read(address + 1)) << 8) + read(address);
}

void MMU::write(u16 address, u8 value) {
    if (blockedByPPU(address)) {
        return;
    }
    if (address <= 0x7FFF) { //cartridge rom
        cartridge->write(address, value);
    } else if (0xA000 <= address && address <= 0xBFFF) { //cartridge ram
        cartridge->write(address, value);
    } else if (address == INPUT_ADDRESS) {
        input->writeInput(value);
    } else if (address == DIV_ADDRESS) {
        memory[address] = 0;
    } else if (address == DISABLE_BOOT_ROM) {
        bootRomDisabled = value; //non-zero disables 
        memory[address] = value;
    } else if (address == SB_ADDRESS) { //Serial port used for debugging
        memory[address] = value;
    } else if (address == SC_ADDRESS) { //Serial port control
        if (value == 0x81) {
            std::cout << (char)memory[SB_ADDRESS] << std::flush;
        }
    } else if (address == DMA_TRSFR_ADDRESS) { // DMA transfer
        u16 startAddress = value << 8;
        memcpy(memory + 0xFE00, memory + startAddress, 160);
        memory[address] = value;
    } else {
        memory[address] = value;
    }
}

//Only use if you know what you're doing
void MMU::writeDirectly(u16 address, u8 value) {
    memory[address] = value;
}

//Only use if you know what you're doing
u8 MMU::readDirectly(u16 address) {
    return memory[address];
}
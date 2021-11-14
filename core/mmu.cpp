#include <iostream>
#include <cstring>
#include <stdio.h>
#include "./mmu.hpp"

bool checkAddressIsValid(u16 address) {
    return address >= 0x0000 && address <= 0xffff;
}

MMU::MMU(Cartridge& cartridge, Input* input) : cartridge(cartridge), input(input) {
    memory = new u8[0x10000];
    memcpy(memory, cartridge.gameRom, cartridge.gameRomSize);
    memcpy(memory, cartridge.bootRom, BOOT_ROM_SIZE);
    memory[INPUT_ADDRESS] = 0xFF; // Input starts high, since high = unpressed
    memory[DIV_ADDRESS] = 0x00;
    memory[TIMA_ADDRESS] = 0x00;
}

MMU::~MMU() {
    delete[] memory;
}

// During mode OAM: CPU cannot access OAM
// During mode VRAM: CPU cannot access VRAM or OAM
// During restricted modes, any attempt to read returns $FF, any attempt to write are ignored
bool MMU::blockedByPPU(u16 address) {
    u8 stat = memory[0xFF41];
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
    if (!checkAddressIsValid(address)) {
        std::cout << "ERROR: Attempted read from forbidden address: " << address << std::endl;
        return 0x00;
    }
    if (blockedByPPU(address)) {
        return 0xFF;
    }
    if (address == INPUT_ADDRESS) {
        return input->readInput();
    }
    return memory[address];
}

u16 MMU::read16Bit(u16 address) {
    if (!(checkAddressIsValid(address) && checkAddressIsValid(address + 1))) {
        std::cout << "ERROR: Attempted read from forbidden address: " << address << std::endl;
        return 0x00;
    }
    if (blockedByPPU(address)) {
        return 0xFF;
    }
    return (u16(memory[address + 1]) << 8) + memory[address];
}

void MMU::write(u16 address, u8 value) {
    if (!checkAddressIsValid(address)) {
        std::cout << "ERROR: Attempted write to forbidden address: " << address << std::endl;
        return;
    }
    if (blockedByPPU(address)) {
        return;
    }

    if (address == INPUT_ADDRESS) {
        input->writeInput(value);
    } else if (address == DIV_ADDRESS) {
        memory[address] = 0;
    } else if (address == ENABLE_BOOT_ROM) {
        if (value != 0) { 
            // Re-map the cartridge
            memcpy(memory, cartridge.gameRom, BOOT_ROM_SIZE);
        } else {
            // Probably not necessary, but it took me 5 seconds to write
            memcpy(memory, cartridge.bootRom, BOOT_ROM_SIZE);
        }
        memory[address] = value;
    } else if (address == SB_ADDRESS) { //Serial port used for debugging
        memory[address] = value;
    } else if (address == SC_ADDRESS) { //Serial port control
        if (value == 0x81) {
            std::cout << (char)memory[SB_ADDRESS] << std::flush;
        }
    } else if (address == 0x2000) {
        // do nothing for now. Eventually MBC
    } else if (address == 0xFF46) { // DMA transfer
        u16 startAddress = value << 8;
        for ( int i = 0; i < 160; i++ ) {
            memory[0xFE00 + i] = memory[startAddress + i];
        }
        memory[address] = value;
    } else {
        memory[address] = value;
    }
}

//Only use if you know what you're doing
void MMU::writeDirectly(u16 address, u8 value) {
    memory[address] = value;
}

u8 MMU::readDirectly(u16 address) {
    return memory[address];
}
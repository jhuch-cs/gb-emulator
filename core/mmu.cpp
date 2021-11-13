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

u8 MMU::read(u16 address) {
    if (!checkAddressIsValid(address)) {
        std::cout << "ERROR: Attempted read from forbidden address: " << address << std::endl;
        return 0x00;
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
    return (u16(memory[address + 1]) << 8) + memory[address];
}

void MMU::write(u16 address, u8 value) {
    if (!checkAddressIsValid(address)) {
        std::cout << "ERROR: Attempted write to forbidden address: " << address << std::endl;
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
    } else {
        memory[address] = value;
    }
}

//Only use if you know what you're doing
void MMU::writeDirectly(u16 address, u8 value) {
    memory[address] = value;
}
#include <iostream>
#include <stdio.h>
#include "./mmu.hpp"

bool checkAddressIsValid(u16 address) {
    return address >= 0x0000 && address <= 0xffff;
}

MMU::MMU(Cartridge cartridge) : cartridge(cartridge) {
    memory = new u8[0x10000];
    memcpy(memory, cartridge.gameRom, cartridge.gameRomSize);
    memcpy(memory, cartridge.bootRom, BOOT_ROM_SIZE);
}

MMU::~MMU() {
    delete[] memory;
}

u8 MMU::read(u16 address) {
    if (!checkAddressIsValid(address)) {
        std::cout << "ERROR: Attempted read from forbidden address: " << address << std::endl;
        return 0x00;
    }
    return memory[address];
}

u16 MMU::read16Bit(u16 address) {
    if (!(checkAddressIsValid(address) && checkAddressIsValid(address + 1))) {
        std::cout << "ERROR: Attempted read from forbidden address: " << address << std::endl;
        return 0x00;
    }
    return (u16(memory[address]) << 8) + memory[ address + 1 ];
}

void MMU::write(u16 address, u8 value) {
    if (!checkAddressIsValid(address)) {
        std::cout << "ERROR: Attempted write to forbidden address: " << address << std::endl;
        return;
    }

    if (address == INPUT_ADDRESS) {
        writeInput(value);
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
    } else {
        memory[address] = value;
    }
}

// For buttons, the `pressed` state is low (0)
void MMU::pressButton(Button button) {
    u8 input = memory[INPUT_ADDRESS];
    input = clearBit(input, static_cast<int>(button) % 2);
    memory[INPUT_ADDRESS] = input;
}

// For buttons, the `unpressed` state is high (1)
void MMU::unpressButton(Button button) {
    u8 input = memory[INPUT_ADDRESS];
    input = setBit(input, static_cast<int>(button) % 2);
    memory[INPUT_ADDRESS] = input;
}

// Only write to the high nibble. Low nibble is input registers.
void MMU::writeInput(u8 value) {
    u8 input = memory[INPUT_ADDRESS];
    input = setHighNibble(input, value);
    memory[INPUT_ADDRESS] = input;
}
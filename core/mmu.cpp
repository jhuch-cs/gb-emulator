#pragma once

#include "./mmu.hpp"

bool checkAddressIsValid(u16 address) {
    return address >= 0x0000 && address < 0xffff;
}

MMU::MMU() {
    memory = new u8[0xffff];
}

MMU::~MMU() {
    delete[] memory;
}

u8 MMU::read(u16 address) {
    if (checkAddressIsValid(address)) {
        return memory[address];
    }
    else throw;
}

u16 MMU::read16Bit(u16 address) {
    if (checkAddressIsValid(address) && checkAddressIsValid(address + 1)) {
        return (u16(memory[address]) << 8) + memory[ address + 1 ];
    }
    else throw;
}

void MMU::write(u16 address, u8 value) {
    if (checkAddressIsValid(address)) {
        memory[address] = value;
    }
    else throw;
}
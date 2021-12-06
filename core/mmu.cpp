#include <iostream>
#include <cstring>
#include <stdio.h>
#include "./mmu.hpp"

MMU::MMU(Cartridge* cartridge, Input* input, u8* bootRom, bool supportsCGB = false) 
    : cartridge(cartridge), input(input), bootRom(bootRom), supportsCGB(supportsCGB) {
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
        return (address >= 0xFE00 && address <= 0xFE9F); // || == VALUE_BG_CRAM || == VALUE_OBJ_CRAM
    }
    // VRAM
    if (stat == 3) {
        return (address >= 0xFE00 && address <= 0xFE9F) || (address >= 0x8000 && address <= 0x97FF); // || == VALUE_BG_CRAM || == VALUE_OBJ_CRAM
    }
    return false;
}

u8 MMU::read(u16 address, bool isPPU) {
    if (!isPPU && blockedByPPU(address)) {
        return 0xFF;
    }
    if (address <= CARTRIDGE_END) { //cartridge rom
        if (!bootRomDisabled) {
            if (supportsCGB) {
                if (address <= BOOT_ROM_SIZE) {
                    return bootRom[address];
                } else if (address <= 0x1FF) { //For whatever reason, this is not mapped to bootRom, but to cartridge
                    return cartridge->read(address);
                } else if (address <= CGB_BOOT_ROM_SIZE) {
                    return bootRom[address];
                }
            } else {
                if (address <= BOOT_ROM_SIZE) {
                    return bootRom[address];
                }
            }
        }
        return cartridge->read(address);
    } else if (address <= VRAM_END) {
        return vram[vramBank][address - VRAM_START];
    } else if (address <= CARTRIDGE_RAM_END) {
        return cartridge->read(address);
    } else if (supportsCGB && address <= WRAM_BANK0_END) {
        return wram[0][address - WRAM_BANK0_END];
    } else if (supportsCGB && address <= WRAM_SWITCH_END) {
        return wram[wramBank][address - WRAM_SWITCH_END];
    } else if (address == INPUT_ADDRESS) {
        return input->readInput();
    } else if (supportsCGB && address == VRAM_BANK) {
        return 0xFE | vramBank; // return the number of the currently loaded VRAM bank in bit 0, and all other bits will be set to 1.
    } else if (supportsCGB && address == WRAM_BANK) {
        return 0xF8 | wramBank;
    } else if (supportsCGB && address == VALUE_BG_CRAM) { 
        return bg_cram[memory[ADDRESS_BG_CRAM] & 0x3F];
    } else if (supportsCGB && address == VALUE_OBJ_CRAM) { 
        return obj_cram[memory[ADDRESS_OBJ_CRAM] & 0x3F];
    }
    return memory[address];
}

u16 MMU::read16Bit(u16 address, bool isPPU) {
    return (u16(read(address + 1, isPPU)) << 8) + read(address, isPPU);
}

void MMU::write(u16 address, u8 value, bool isPPU) {
    if (!isPPU && blockedByPPU(address)) {
        return;
    }
    if (address <= CARTRIDGE_END) { //cartridge rom
        cartridge->write(address, value);
    } else if (address <= VRAM_END) {
        vram[vramBank][address - VRAM_START] = value;
    } else if (address <= CARTRIDGE_RAM_END) { //cartridge ram
        cartridge->write(address, value);
    } else if (supportsCGB && address <= WRAM_BANK0_END) {
        wram[0][address - WRAM_BANK0_END] = value;
    } else if (supportsCGB && address <= WRAM_SWITCH_END) {
        wram[wramBank][address - WRAM_SWITCH_END] = value;
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
        for (int i = 0; i < 160; i++) {
            memory[0xFE00 + i] = read(startAddress + i);
        }
        memory[address] = value;
    } else if (supportsCGB && address == VRAM_BANK) {
        vramBank = value & 0b1; //Only bit 0 matters, all other bits are ignored.
    } else if (supportsCGB && address == WRAM_BANK) {
        wramBank = value & 0x07; //only bottom 3 bits
        if (!wramBank) { wramBank++; } //can't be zero
    } else if (supportsCGB && address == VALUE_BG_CRAM) { 
        bg_cram[memory[ADDRESS_BG_CRAM] & 0x3F] = value;
        if (memory[ADDRESS_BG_CRAM] & 0b10000000) {
            memory[ADDRESS_BG_CRAM]++; //FIXME: Does this wrap at bit 5 or 7?
        }
    } else if (supportsCGB && address == VALUE_OBJ_CRAM) { 
        obj_cram[memory[ADDRESS_OBJ_CRAM] & 0x3F] = value;
        if (memory[ADDRESS_OBJ_CRAM] & 0b10000000) {
            memory[ADDRESS_OBJ_CRAM]++; //FIXME: Does this wrap at bit 5 or 7?
        }
    } else if (supportsCGB && address == HDMA5_OPTS) {
        memory[HDMA5_OPTS] = value;
        if (value & 0b10000000) {
            doGDMA();
        } else {
            doHDMA();
        }
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

void MMU::doGDMA() {
    u16 source = ((u16)memory[HDMA1_SOURCE]) << 8 | memory[HDMA2_SOURCE];
    u16 dest   = ((u16)memory[HDMA3_DEST])   << 8 | memory[HDMA4_DEST];

    source &= 0xFFF0; // ignore bottom 4 bits

    dest &= 0x1FF0;   // ignore bits 15-13, 3-0
    dest |= 0x8000;   // prevent writes to cartridge

    u16 length = memory[HDMA5_OPTS] & 0x7F; // lower 7 bits specify Transfer Length
    length = (length / 0x10) - 1; // divided by 10h, minus 1

    for (u16 i = 0; i < length; i++) {
        write(dest++, read(source++));
    }

    memory[HDMA5_OPTS] = 0xFF;

    // TODO: Some implementations reset all HDMA registers. Why?
}

void MMU::doHDMA() {
    // HDMA differs from GDMA from a timing perspective (during HBLANK)
    // and might not be necessary for our purposes (show off Pokemon)

    // It also can be stopped. It can also have its status read.
    // https://gbdev.io/pandocs/CGB_Registers.html#bit-7--1---hblank-dma
    doGDMA(); 
}
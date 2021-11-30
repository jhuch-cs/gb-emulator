#include <iostream>
#include <cstring>
#include <stdio.h>
#include "./mmu.hpp"

MMU::MMU(Cartridge* cartridge, Input* input, u8* bootRom, PPU_Shared_Mem* PPU_Shared, CPU_Shared_Mem* CPU_Shared, Timer_Shared_Mem* Timer_Shared) 
    : cartridge(cartridge), input(input), bootRom(bootRom), PPU_Shared(PPU_Shared), CPU_Shared(CPU_Shared), Timer_Shared(Timer_Shared) {
    memory[INPUT_ADDRESS] = 0xFF; // Input starts high, since high = unpressed
}

MMU::~MMU() {}

// During mode OAM: CPU cannot access OAM
// During mode VRAM: CPU cannot access VRAM or OAM
// During restricted modes, any attempt to read returns $FF, any attempt to write are ignored
bool MMU::blockedByPPU(u16 address) {
    u8 stat = PPU_Shared->stat;
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
    switch (address) {
        case IF_ADDRESS:
            return CPU_Shared->interrupt_flags;
        case IE_ADDRESS: 
            return CPU_Shared->interrupt_enable;
        case DIV_ADDRESS:
            return Timer_Shared->div;
        case TIMA_ADDRESS:
            return Timer_Shared->tima;
        case TMA_ADDRESS:
            return Timer_Shared->tma;
        case TAC_ADDRESS:
            return Timer_Shared->tac;
        case INPUT_ADDRESS:
            return input->readInput();
        case LCDC:
            return PPU_Shared->lcdc;
        case STAT:
            return PPU_Shared->stat;
        case SCY:
            return PPU_Shared->scy;
        case SCX:
            return PPU_Shared->scx;
        case LY:
            return PPU_Shared->ly;
        case LYC:
            return PPU_Shared->lyc;
        case DMA:
            return PPU_Shared->dma;
        case BGP: 
            return PPU_Shared->bgp;
        case OBP0:
            return PPU_Shared->obp0;
        case OBP1:
            return PPU_Shared->obp1;
        case WY:
            return PPU_Shared->wy;
        case WX:
            return PPU_Shared->wx;
    }
    if (address <= 0x7FFF) { //cartridge rom
        if (address < BOOT_ROM_SIZE && !bootRomDisabled) {
            return bootRom[address];
        }
        return cartridge->read(address);
    } else if (address <= VRAM_END) {
        return PPU_Shared->vram[address - VRAM_START];
    } else if (OAM_START <= address && address <= OAM_END) {
        return PPU_Shared->oam[address - OAM_START];
    } else if (0xA000 <= address && address <= 0xBFFF) { //cartridge ram
        return cartridge->read(address);
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
    switch (address) {
        case IF_ADDRESS:
            CPU_Shared->interrupt_flags = value;
            return;
        case IE_ADDRESS:
            CPU_Shared->interrupt_enable = value;
            return;
        case DIV_ADDRESS:
            Timer_Shared->div = 0;
        case TIMA_ADDRESS:
            Timer_Shared->tima = value;
        case TMA_ADDRESS:
            Timer_Shared->tma = value;
        case TAC_ADDRESS:
            Timer_Shared->tac = value;
        case INPUT_ADDRESS:
            input->writeInput(value);
            return;
        case DISABLE_BOOT_ROM:
            bootRomDisabled = value; //non-zero disables 
            memory[DISABLE_BOOT_ROM] = value;
            return;
        case SB_ADDRESS:
            memory[SB_ADDRESS] = value;
            return;
        case SC_ADDRESS:
            if (value == 0x81) {
                std::cout << (char)memory[SB_ADDRESS] << std::flush;
            }
            return;
        case DMA_TRSFR_ADDRESS: {
            u16 startAddress = value << 8;
            memcpy(PPU_Shared->oam, memory + startAddress, 160);
            PPU_Shared->dma = value;
        } return;
        case LCDC:
            PPU_Shared->lcdc = value;
            return;
        case STAT:
            PPU_Shared->stat = value;
            return;
        case SCY:
            PPU_Shared->scy = value;
            return;
        case SCX:
            PPU_Shared->scx = value;
            return;
        case LY:
            PPU_Shared->ly = value;
            return;
        case LYC:
            PPU_Shared->lyc = value;
            return;
        case BGP: 
            PPU_Shared->bgp = value;
            return;
        case OBP0:
            PPU_Shared->obp0 = value;
            return;
        case OBP1:
            PPU_Shared->obp1 = value;
            return;
        case WY:
            PPU_Shared->wy = value;
            return;
        case WX:
            PPU_Shared->wx = value;
            return;
    }
    if (address <= 0x7FFF) { //cartridge rom
        cartridge->write(address, value);
    } else if (address <= VRAM_END) {
        PPU_Shared->vram[address - VRAM_START] = value;
    } else if (OAM_START <= address && address <= OAM_END) {
        PPU_Shared->oam[address - OAM_START] = value;
    }  else if (0xA000 <= address && address <= 0xBFFF) { //cartridge ram
        cartridge->write(address, value);
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
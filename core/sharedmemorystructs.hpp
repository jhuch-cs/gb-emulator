#pragma once

#include "./util.hpp"

const u16 VRAM_START = 0x8000;
const u16 VRAM_END = 0x9FFF;
const u16 OAM_START = 0xFE00;
const u16 OAM_END = 0xFE9F;

const u16 LCDC = 0xFF40;
const u16 STAT = 0xFF41;
const u16 SCY = 0xFF42;
const u16 SCX = 0xFF43;
const u16 LY = 0xFF44;
const u16 LYC = 0xFF45;
const u16 DMA = 0xFF46;
const u16 BGP = 0xFF47;
const u16 OBP0 = 0xFF48;
const u16 OBP1 = 0xFF49;
const u16 WY = 0xFF4A;
const u16 WX = 0xFF4B;

struct PPU_Shared_Mem {
    u8 vram[0x2000] = {};

    u8 oam[160] = {};

    u8 lcdc = 0x00;
    u8 stat = 0x00;
    u8 scy  = 0x00;
    u8 scx  = 0x00;
    u8 ly   = 0x00;
    u8 lyc  = 0x00;
    u8 dma  = 0x00;
    u8 bgp  = 0x00;
    u8 obp0 = 0x00;
    u8 obp1 = 0x00;
    u8 wy   = 0x00;
    u8 wx   = 0x00;
};

struct CPU_Shared_Mem {
    u8 interrupt_flags  = 0x00;
    u8 interrupt_enable = 0x00;
};

struct Timer_Shared_Mem {
    u8 div  = 0x00;
    u8 tima = 0x00;
    u8 tma  = 0x00;
    u8 tac  = 0x00;
};
#pragma once

#include "./util.hpp"
#include "./ppu.hpp"

// PPU::PPU(CPU& cpu, MMU& mmu) {
//   PPU::cpu = cpu;
//   PPU::mmu = mmu;
// }

u8* PPU::getFrameBuffer() {
  return frameBuffer;
}

void PPU::step(u8 cpuCyclesElapsed) {

  // get updated lcd control
  u8 lcdCtrl = mmu.read(lcdc);
  // get updated lcd status
  u8 lcdStat = mmu.read(stat);
  // get ly (current scanline)
  u8 scanline = mmu.read(ly);

  //...

  // for the amount of cycles:
  // switch based on current mode

  // check current scanline == 144, then enter VBLANK
  // reset scanline to 0 if > 153 (end of VBLANK)
  // when less than 144, call drawScanLine(lcd_ctrl)
}

// PPU mode typically goes from OAM -> VRAM -> HBLANK, repeating until VBLANK (aka mode 1)
// 456 clocks (114 cycles) to draw one scanline
  // OAM - 80 clocks (20 cycles)
  // VRAM - 172-289 clocks (43-72 cycles) [set default to start at 172?]
  // HBLANK - 87-204 clocks (22-51) cycles) (depending on prev) [set default to start at 289?]
void PPU::drawScanLine(u8 lcdCtrl) {

  // check if lcd is enabled (or is this done in step()?)

  if (checkBit(lcdCtrl, 0)) {
    renderTiles(lcdCtrl);
  }
  if (checkBit(lcdCtrl, 1)) {
    renderSprites();
  }
}

// Tile Data in one of two locations: (controled by LCDC Bit 4)
  // 0X800-0X8FFF (unsigned numbers from 0 - 255)
  // 0X8800-0X97FF (singed nubmers from -128 - 127)
// Tile Map in 0x9800-0x9BFF or 0x9C00-0X9FFF
  // Window tile map area	determined by Bit 6 in LCDC -> 0=9800-9BFF, 1=9C00-9FFF
  // Backgrond tile map area determined by Bit 3 in LCDC -> 0=9800-9BFF, 1=9C00-9FFF
void PPU::renderTiles(u8 lcdCtrl) {

  u8 scrollY = mmu.read(scy);
  u8 scrollX = mmu.read(scx);
  u8 windowY = mmu.read(wy);
  u8 windowX = mmu.read(wx) - 7;

  //lcdCtrl
  // Bit 6	Window tile map area	0=9800-9BFF, 1=9C00-9FFF
  // Bit 5	Window enable	0=Off, 1=On
  // Bit 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
  // Bit 3	BG tile map area	0=9800-9BFF, 1=9C00-9FFF

}

// OAM table: 0xFE00 - 0XFE9F
// OAM entries:
  // Byte 0 - Y pos + 16
  // Byte 1 - X pos + 8
  // Byte 2 - Tile Index
    // 8x8 mode (LCDC bit 2 = 0), unsigned val 0x00 - 0xFF
    // 8x16 mode (LCDC bit 2 = 1), every two tiles form a sprite (top then bottom)
  // Byte 3 - Attributes/Flags
    // Bit 7   BG and Window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
    // Bit 6   Y flip          (0=Normal, 1=Vertically mirrored)
    // Bit 5   X flip          (0=Normal, 1=Horizontally mirrored)
    // Bit 4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
    // Bit 3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
    // Bit 2-0 Palette number  **CGB Mode Only**     (OBP0-7)
void PPU::renderSprites() {

  // check if sprites are 8x8 or 8x16
  u8 lcdCtrl = mmu.read(lcdc);
  bool use8x16 = checkBit(lcdCtrl, 2);

  // can render up to 40 sprites, iterate through OAM table
  for (int index = 0; index < 40; index++) {
    
  }
}

// During mode VRAM: OAM, HBLANK, and VBLANK can accessible
// During mode OAM: HBLANK and VBLANK accessible
  // During restricted modes, any attempt to read returns $FF, any attempt to write are ignored
// PPU::cpuReadVRAM()
// PPU::cpuWriteVRAM()
// PPU::cpuReadOAM()
// PPU::cpuWriteOAM()

// Helper Functions
bool checkBit(u8 value, u8 index) { 
  return (value & (1 << index)) != 0;
}
#pragma once

#include "./util.hpp"
#include "./ppu.hpp"

// PPU::PPU(CPU& cpu, MMU& mmu) {
//   this->cpu = cpu;
//   this->mmu = mmu;
//   mode = OAM;
//   cyclesLeft = 0;
// }

// LCDC (0xFF40) - LCD Control
  // (see lcdc helper functions)
u16 PPU::get_lcdc() { return mmu.read(0xFF40); }
// STAT (0xFF41) - LCD Status
  // Bit 6 - LYC=LY STAT Interrupt source         (1=Enable) (Read/Write)
  // Bit 5 - Mode 2 OAM STAT Interrupt source     (1=Enable) (Read/Write)
  // Bit 4 - Mode 1 VBlank STAT Interrupt source  (1=Enable) (Read/Write)
  // Bit 3 - Mode 0 HBlank STAT Interrupt source  (1=Enable) (Read/Write)
  // Bit 2 - LYC=LY Flag                          (0=Different, 1=Equal) (Read Only)
  // Bit 1-0 - Mode Flag                          (Mode 0-3, see below) (Read Only)
  //           0: HBlank
  //           1: VBlank
  //           2: Searching OAM
  //           3: Transferring Data to LCD Controller
u16 PPU::get_stat() { return mmu.read(0xFF41); }
// ScrollY (0xFF42) - y pos of background
u16 PPU::get_scy() { return mmu.read(0xFF42); }
// ScrollX (0xFF43) - x pos of background
u16 PPU::get_scx() { return mmu.read(0xFF43); }
// LY (0xFF44) - LCD Y Coordinate (aka current scanline)
// Holds values 0-153, with 144-153 indicating VBLANK
u16 PPU::get_ly() { return mmu.read(0xFF44); }
// LYC (0xFF45) - LY Compare
u16 PPU::get_lyc() { return mmu.read(0xFF45); }
// DMA (0xFF46) - DMA Transfer and Start (160 cycles?)
u16 PPU::get_dma() { return mmu.read(0xFF46); }
// bgp (0xFF47) - BG palette data
  // Bit 7-6 Color for index 3
  // Bit 5-4 Color for index 2
  // Bit 3-2 Color for index 1
  // Bit 1-0 Color for index 0
u16 PPU::get_bgp() { return mmu.read(0xFF47); }
// obp0 (0xFF48) - OBJ palette 0 dataQ
// Just like bgp but bits 1-0 ignored because color index 0 is transparent for sprites
u16 PPU::get_obp0() { return mmu.read(0xFF48); }
// obp1 (0XFF49) - OBJ palette 1 data
u16 PPU::get_obp1() { return mmu.read(0xFF49); }

u16 PPU::get_wy() { return mmu.read(0xFF4A); }
u16 PPU::get_wx() { return mmu.read(0xFF4B); }

u8* PPU::getFrameBuffer() { return frameBuffer; }

// Define setters as needed


// lcdc register helper functions 
// Bit 7	LCD and PPU enable	0=Off, 1=On
bool PPU::isLCDEnabled() { return checkBit(get_lcdc(), 7); }
// Bit 6	Window tile map area	0=9800-9BFF, 1=9C00-9FFF
u16 PPU::windowTileMapArea() { return checkBit(get_lcdc(), 6) ? 0x9C00 : 0x9800; }
// Bit 5	Window enable	0=Off, 1=On
bool PPU::isWindowEnabled() { return checkBit(get_lcdc(), 5); }
// Bit 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
u16 PPU::tileDataArea() { return checkBit(get_lcdc(), 4) ? 0x800 : 0x8800; }
// Bit 3	BG tile map area	0=9800-9BFF, 1=9C00-9FFF
u16 PPU::bgTileMapArea() { return checkBit(get_lcdc(), 3) ? 0x9C00 : 0x9800; }
// Bit 2	OBJ size	0=8x8, 1=8x16
bool PPU::isObj8x16() { return checkBit(get_lcdc(), 2); }
// Bit 1	OBJ enable	0=Off, 1=On
bool PPU::isObjEnabled() { return checkBit(get_lcdc(), 1); }
// Bit 0	BG and Window enable/priority	0=Off, 1=On
bool PPU::isBgWinEnabled() { return checkBit(get_lcdc(), 0); }

void PPU::step(u8 cpuCyclesElapsed) {

  cyclesLeft += cpuCyclesElapsed / 2; // is this corect behavior or should remaining cycles not carry over?

  // switch based on current mode
  // TODO: need a way to check interupts / set interupts for the CPU, maybe by public access to some methods in CPU..?
    // Bit 6 - LYC=LY STAT Interrupt source         (1=Enable) (Read/Write)
    // Bit 5 - Mode 2 OAM STAT Interrupt source     (1=Enable) (Read/Write)
    // Bit 4 - Mode 1 VBlank STAT Interrupt source  (1=Enable) (Read/Write)
    // Bit 3 - Mode 0 HBlank STAT Interrupt source  (1=Enable) (Read/Write)
  // this will be done depending on the mode
  switch(mode) {
    case OAM:
      if (cyclesLeft >= OAM_CLOCKS) {

      }
      break;
    case VRAM:
      if (cyclesLeft >= VRAM_CLCOKS) {

      }
      break;
    case HBLANK:
      if (cyclesLeft >= HBLANK_CLOCKS) {

      }
      break;
    case VBLANK:
      if (cyclesLeft >= VBLANK_CLOCKS) {

      }
      break;
  }

  // check current scanline == 144, then enter VBLANK
  // reset scanline to 0 if > 153 (end of VBLANK)
  // when less than 144, call drawScanLine(lcd_ctrl)
}

// PPU mode typically goes from OAM -> VRAM -> HBLANK, repeating until VBLANK (aka mode 1)
// 456 clocks (114 cycles) to draw one scanline
  // OAM - 80 clocks (20 cycles)
  // VRAM - 172-289 clocks (43-72 cycles) [set default to start at 172?]
  // HBLANK - 87-204 clocks (22-51) cycles) (depending on prev) [set default to start at 289?]
void PPU::drawScanLine() {

  if (!isLCDEnabled()) { return; }

  if (isBgWinEnabled()) {
    renderTiles();
  }
  if (isObjEnabled()) {
    renderSprites();
  }
}

// Tile Data in one of two locations: (controled by LCDC Bit 4)
  // 0X800-0X8FFF (unsigned numbers from 0 - 255)
  // 0X8800-0X97FF (singed nubmers from -128 - 127)
// Tile Map in 0x9800-0x9BFF or 0x9C00-0X9FFF
  // Window tile map area	determined by Bit 6 in LCDC -> 0=9800-9BFF, 1=9C00-9FFF
  // Backgrond tile map area determined by Bit 3 in LCDC -> 0=9800-9BFF, 1=9C00-9FFF
void PPU::renderTiles() {

  u8 scrollY = get_scy();
  u8 scrollX = get_scx();
  u8 windowY = get_wy();
  u8 windowX = get_wx() - 7;

  // LCDC Bit 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
  u16 tileData = tileDataArea();

  // LCDC Bit 3	BG tile map area	0=9800-9BFF, 1=9C00-9FFF
  u16 bgMap = bgTileMapArea();

  // LCDC Bit 5	Window enable	0=Off, 1=On
  // Check if window's Y position is within the current scanline
  // LCDC Bit 6	Window tile map area	0=9800-9BFF, 1=9C00-9FFF
  u16 winMap;
  if (isWindowEnabled() && get_wy() <= get_ly()) {
    winMap = windowTileMapArea();
  }

  // draw current line of pixels
  for (int i = 0; i < LCD_HEIGHT; i++) {

  }
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

  bool use8x16 = isObj8x16();

  // ...

  // can render up to 40 sprites, iterate through OAM table
  for (int i = 0; i < 40; i++) {
    u8 spriteIndex = i*4;
    u8 yPos = mmu.read(OAM_TABLE + spriteIndex) - 16; // should this be - 16 for true pos?
    u8 xPos = mmu.read(OAM_TABLE + spriteIndex + 1) - 8; // should this be - 8 for true pos?
    u8 tileIndex = mmu.read(OAM_TABLE + spriteIndex + 2);
    u8 attr = mmu.read(OAM_TABLE + spriteIndex + 3);

    // check attributes?
  }
}
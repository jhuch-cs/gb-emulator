#pragma once

#include "./util.hpp"
#include "./ppu.hpp"

// PPU::PPU(CPU& cpu, MMU& mmu) {
//   this->cpu = cpu;
//   this->mmu = mmu;
//   mode = OAM;
//   cyclesLeft = 0;

//   frameBuffer = new u8**[LCD_HEIGHT];
//   for (int i=0; i<LCD_HEIGHT; i++) {
//     frameBuffer[i] = new u8*[LCD_WIDTH];
//     for (int j=0; j<LCD_WIDTH; j++) {
//       frameBuffer[i][j] = new u8[3];
//     }
//   }
// }

// LCDC (0xFF40) - LCD Control
  // (see lcdc helper functions)
u8 PPU::get_lcdc() { return mmu.read(LCDC); }
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
u8 PPU::get_stat() { return mmu.read(STAT); }
// ScrollY (0xFF42) - y pos of background
u8 PPU::get_scy() { return mmu.read(SCY); }
// ScrollX (0xFF43) - x pos of background
u8 PPU::get_scx() { return mmu.read(SCX); }
// LY (0xFF44) - LCD Y Coordinate (aka current scanline)
// Holds values 0-153, with 144-153 indicating VBLANK
u8 PPU::get_ly() { return mmu.read(LY); }
// LYC (0xFF45) - LY Compare
u8 PPU::get_lyc() { return mmu.read(LYC); }
// DMA (0xFF46) - DMA Transfer and Start (160 cycles?)
u8 PPU::get_dma() { return mmu.read(DMA); }
// bgp (0xFF47) - BG palette data
  // Bit 7-6 Color for index 3
  // Bit 5-4 Color for index 2
  // Bit 3-2 Color for index 1
  // Bit 1-0 Color for index 0
u8 PPU::get_bgp() { return mmu.read(BGP); }
// obp0 (0xFF48) - OBJ palette 0 dataQ
// Just like bgp but bits 1-0 ignored because color index 0 is transparent for sprites
u8 PPU::get_obp0() { return mmu.read(OBP0); }
// obp1 (0XFF49) - OBJ palette 1 data
u8 PPU::get_obp1() { return mmu.read(OBP1); }

u8 PPU::get_wy() { return mmu.read(WY); }
u8 PPU::get_wx() { return mmu.read(WX); }

u8*** PPU::getFrameBuffer() { 
  return frameBuffer;
}

// Define setters as needed
void PPU::set_ly(u8 ly) { mmu.write(LY, ly); }

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

  if (!isLCDEnabled()) { return; }

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
        cyclesLeft -= OAM_CLOCKS;
        
        // update stat register for VRAM mode
        u8 stat = get_stat();
        stat = setBit(stat, 0);
        stat = setBit(stat, 1);
        mmu.write(STAT, stat);
        mode = VRAM;
      }
      break;
    case VRAM:
      if (cyclesLeft >= VRAM_CLOCKS) {
        cyclesLeft -= VRAM_CLOCKS;

        drawScanLine();
        
        mode = HBLANK;
        u8 stat = get_stat();
        stat = clearBit(stat, 0);
        stat = clearBit(stat, 1);
        mmu.write(STAT, stat);

        // check for HBLANK interrupt
        if (checkBit(get_stat(), 3)) {
          // signal cpu interrupt?
        }
      }
      break;
    case HBLANK:
      if (cyclesLeft >= HBLANK_CLOCKS) {
        cyclesLeft -= HBLANK_CLOCKS;

        u8 scanline = get_ly();

        // check lyc interupt // should this be somewhere else?
        bool lyc = (scanline == get_lyc());
        if (checkBit(get_stat(), 6) && lyc) {
          // signal cpu interrupt?
        }
        
        // increment current line
        scanline++;
        mmu.write(LY, scanline);

        // check current scanline == 144, then enter VBLANK, else enter OAM to prepare to draw another line
        if (scanline == 144) {
          mode = VBLANK;
          u8 stat = get_stat();
          stat = lyc ? setBit(stat, 2) : clearBit(stat, 2);
          stat = setBit(stat, 0);
          stat = clearBit(stat, 1);
          mmu.write(STAT, stat);

          if (checkBit(get_stat(), 4)) {
            // signal cpu interrupt?
          }
        } else {
          mode = OAM;
          u8 stat = get_stat();
          stat = lyc ? setBit(stat, 2) : clearBit(stat, 2);
          stat = clearBit(stat, 0);
          stat = setBit(stat, 1);
          mmu.write(STAT, stat);

          if (checkBit(get_stat(), 5)) {
            // signal cpu interrupt?
          }
        }
      }
      break;
    case VBLANK:
      if (cyclesLeft >= VBLANK_CLOCKS) {
        cyclesLeft -= VBLANK_CLOCKS;

        // increment current line
        u8 scanline = get_ly() + 1;
        mmu.write(LY, scanline);

        // reset scanline to 0 if > 153 (end of VBLANK)
        if (scanline == 154) {

          // reset scanline to 0
          mmu.write(LY, 0);
          
          mode = OAM;
          u8 stat = get_stat();
          stat = clearBit(stat, 0);
          stat = setBit(stat, 1);
          mmu.write(STAT, stat);

          if (checkBit(get_stat(), 5)) {
            // signal cpu interrupt?
          }
        }
      }
      break;
  }
}

// PPU mode typically goes from OAM -> VRAM -> HBLANK, repeating until VBLANK (aka mode 1)
// 456 clocks (114 cycles) to draw one scanline
  // OAM - 80 clocks (20 cycles)
  // VRAM - 172-289 clocks (43-72 cycles) [set default to start at 172?]
  // HBLANK - 87-204 clocks (22-51) cycles) (depending on prev) [set default to start at 289?]
void PPU::drawScanLine() {

  // Should rednering background and window be done separately for simplicity sake?
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
  bool unsig = checkBit(get_lcdc(), 4);

  // Check if window's Y position is within the current scanline and window is enabled
  // Tetris doesn't use a window, so this should be just set to the bgTileMapAreaa()
  u16 tileMap;
  bool winEnabled = (isWindowEnabled() && get_wy() <= get_ly());
  if (!winEnabled) {
    tileMap = bgTileMapArea();
  } else {
    tileMap = windowTileMapArea();
  }

  u8 currentLine = get_ly();

  // calculate current row of tiles we are on
  u8 yPos = 0;
  if (!winEnabled) {
    yPos = scrollY + currentLine;
  } else {
    yPos = currentLine - windowY;
  }

  // calculate which row of pixels in the above tile we are on (32 tiles vertical, 8 pixels per tile)
  u16 currPixelRow = ((u8)(yPos/8)*32);

  // draw current line of pixels
  for (int i = 0; i < LCD_WIDTH; i++) {
    u8 xPos = i + scrollX;

    // same as above for x offset (if window is enabled)
    if (winEnabled) {
      if (i >= windowX) {
        xPos = i - windowX;
      }
    }
    
    u16 currPixelCol = (xPos/8);
    u16 tileAddress = tileMap + currPixelRow + currPixelCol;

    s16 tileNum;
    if (unsig) {
      tileNum = (u8)mmu.read(tileAddress);
    } else {
      tileNum = (s8)mmu.read(tileAddress);
    }

    // adjust for unsigned by using 128 size offset
    u16 tileLoc = tileData;
    if (unsig) {
      tileLoc += (tileNum * 16);
    } else {
      tileLoc += ((tileNum + 128) * 16);
    }

     u8 line = yPos % 8;
     line *= 2; // two bytes of mem per line
     u8 byte1 = mmu.read(tileLoc + line);
     u8 byte2 = mmu.read(tileLoc + line + 1);

     int colorBit = xPos % 8;
     colorBit -= 7;
     colorBit *= -1;

     int colorId = checkBit(byte2, colorBit);
     colorId <<= 1;
     colorId |= checkBit(byte1, colorBit);

     int color = getcolor(colorId, BGP) ;
     
     u8 red = 0;
     u8 green = 0;
     u8 blue = 0;
     switch(color) {
       case 0: red = 155; green = 188 ; blue = 15; break;   // WHITE = 0  #9bbc0f RGB: 155, 188, 15
       case 1:red = 139; green = 172 ; blue = 15; break;    // LIGHT_GRAY = 1 #8bac0f RGB: 139, 172, 15
       case 2: red = 48; green = 98 ; blue = 48; break;     // DARK_GRAY = 2  #306230 RGB: 48, 98, 48
       default: red = 15; green = 56; blue = 15; break;     // BLACK = 3  #0f380f RGB: 15, 56, 15
     }

     frameBuffer[i][currentLine][0] = red ;
     frameBuffer[i][currentLine][1] = green ;
     frameBuffer[i][currentLine][2] = blue ;
  }
}

int PPU::getcolor(int id, u16 palette) {
    u8 palette = mmu.read(palette);
    int hi = 2 * id + 1;
    int lo = 2 * id;
    int bit1 = (palette >> hi) & 1;
    int bit0 = (palette >> lo) & 1;

    return (bit1 << 1) | bit0;
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

  return; // TODO: implement this

  bool use8x16 = isObj8x16();
  int objSize = use8x16 ? 16 : 8;

  // can render up to 40 sprites, iterate through OAM table
  for (int i = 0; i < 40; i++) {
    u8 spriteIndex = i*4;
    u8 yPos = mmu.read(OAM_TABLE + spriteIndex) - 16; // should this be - 16 for true pos?
    u8 xPos = mmu.read(OAM_TABLE + spriteIndex + 1) - 8; // should this be - 8 for true pos?
    u8 tileIndex = mmu.read(OAM_TABLE + spriteIndex + 2);
    u8 attr = mmu.read(OAM_TABLE + spriteIndex + 3);

    // check sprite attributes
    const bool bgOverObj = checkBit(attr, 7);
    const bool yFlip = checkBit(attr, 6);
    const bool xFlip = checkBit(attr, 5);
    const u16 pallete = checkBit(attr, 4) ? OBP1 : OBP0;

    // get current scanline
    u8 scanline = get_ly();

    // draw row of pixels in sprite if sprite intercepts scanline
    if (scanline >= yPos && scanline < (yPos + objSize)) {

    }
  }
}
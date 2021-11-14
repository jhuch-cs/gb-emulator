#include "./ppu.hpp"

// Static for memory-stuff
u8* PPU::frameBuffer = new u8[LCD_HEIGHT * LCD_WIDTH * 3];

PPU::PPU(MMU& mmu, CPU& cpu) : mmu(mmu), cpu(cpu) {
  mode = OAM;
  cyclesLeft = 0;
}

// LCDC (0xFF40) - LCD Control
  // (see lcdc helper functions)
u8 PPU::get_lcdc() { return mmu.readDirectly(LCDC); }
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
u8 PPU::get_stat() { return mmu.readDirectly(STAT); }
// ScrollY (0xFF42) - y pos of background
u8 PPU::get_scy() { return mmu.readDirectly(SCY); }
// ScrollX (0xFF43) - x pos of background
u8 PPU::get_scx() { return mmu.readDirectly(SCX); }
// LY (0xFF44) - LCD Y Coordinate (aka current scanline)
// Holds values 0-153, with 144-153 indicating VBLANK
u8 PPU::get_ly() { return mmu.readDirectly(LY); }
// LYC (0xFF45) - LY Compare
u8 PPU::get_lyc() { return mmu.readDirectly(LYC); }
// DMA (0xFF46) - DMA Transfer and Start (160 cycles?)
u8 PPU::get_dma() { return mmu.readDirectly(DMA); }
// bgp (0xFF47) - BG palette data
  // Bit 7-6 Color for index 3
  // Bit 5-4 Color for index 2
  // Bit 3-2 Color for index 1
  // Bit 1-0 Color for index 0
u8 PPU::get_bgp() { return mmu.readDirectly(BGP); }
// obp0 (0xFF48) - OBJ palette 0 dataQ
// Just like bgp but bits 1-0 ignored because color index 0 is transparent for sprites
u8 PPU::get_obp0() { return mmu.readDirectly(OBP0); }
// obp1 (0XFF49) - OBJ palette 1 data
u8 PPU::get_obp1() { return mmu.readDirectly(OBP1); }

u8 PPU::get_wy() { return mmu.readDirectly(WY); }
u8 PPU::get_wx() { return mmu.readDirectly(WX); }

u8* PPU::getFrameBuffer() { return frameBuffer; }

// Define setters as needed
void PPU::set_ly(u8 ly) { mmu.writeDirectly(LY, ly); }

// lcdc register helper functions 
// Bit 7	LCD and PPU enable	0=Off, 1=On
bool PPU::isLCDEnabled() { return checkBit(get_lcdc(), 7); }
// Bit 6	Window tile map area	0=9800-9BFF, 1=9C00-9FFF
u16 PPU::windowTileMapArea() { return checkBit(get_lcdc(), 6) ? 0x9C00 : 0x9800; }
// Bit 5	Window enable	0=Off, 1=On
bool PPU::isWindowEnabled() { return checkBit(get_lcdc(), 5); }
// Bit 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
u16 PPU::tileDataArea() { return checkBit(get_lcdc(), 4) ? 0x8000 : 0x8800; }
// Bit 3	BG tile map area	0=9800-9BFF, 1=9C00-9FFF
u16 PPU::bgTileMapArea() { return checkBit(get_lcdc(), 3) ? 0x9C00 : 0x9800; }
// Bit 2	OBJ size	0=8x8, 1=8x16
bool PPU::isObj8x16() { return checkBit(get_lcdc(), 2); }
// Bit 1	OBJ enable	0=Off, 1=On
bool PPU::isObjEnabled() { return checkBit(get_lcdc(), 1); }
// Bit 0	BG and Window enable/priority	0=Off, 1=On
bool PPU::isBgWinEnabled() { return checkBit(get_lcdc(), 0); }

void PPU::step(u8 cpuCyclesElapsed) {

  if (!isLCDEnabled()) { 
    return; 
  }

  cyclesLeft += cpuCyclesElapsed;

  // switch based on current mode
    // Bit 6 - LYC=LY STAT Interrupt source         (1=Enable) (Read/Write)
    // Bit 5 - Mode 2 OAM STAT Interrupt source     (1=Enable) (Read/Write)
    // Bit 4 - Mode 1 VBlank STAT Interrupt source  (1=Enable) (Read/Write)
    // Bit 3 - Mode 0 HBlank STAT Interrupt source  (1=Enable) (Read/Write)
  // this will be done depending on the mode
  switch(mode) {
    case OAM:
      if (cyclesLeft >= OAM_CLOCKS) {
        cyclesLeft -= OAM_CLOCKS;
        
        mode = VRAM;
        u8 stat = get_stat();
        stat = setBit(stat, 0);
        stat = setBit(stat, 1);
        mmu.writeDirectly(STAT, stat);
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
        mmu.writeDirectly(STAT, stat);

        // HBLANK stat interrupt
        if (checkBit(get_stat(), 3)) {
          cpu.requestInterrupt(Interrupt::LCD_STAT);
        }
      }
      break;
    case HBLANK:
      if (cyclesLeft >= HBLANK_CLOCKS) {
        cyclesLeft -= HBLANK_CLOCKS;

        // get and increment scanline
        u8 scanline = get_ly() + 1;
        mmu.writeDirectly(LY, scanline);

        // check lyc interupt
        checkLYC(scanline);
        
        // check current scanline >= 144, then enter VBLANK, else enter OAM to prepare to draw another line
        if (scanline >= 144) {
          mode = VBLANK;
          cpu.requestInterrupt(VBLANK_INT); 
          u8 stat = get_stat();
          stat = setBit(stat, 0);
          stat = clearBit(stat, 1);
          mmu.writeDirectly(STAT, stat);

          // VBLANK stat interrupt 
          if (checkBit(get_stat(), 4)) {
            cpu.requestInterrupt(Interrupt::LCD_STAT);
          }
        } else {
          mode = OAM;
          u8 stat = get_stat();
          stat = clearBit(stat, 0);
          stat = setBit(stat, 1);
          mmu.writeDirectly(STAT, stat);

          // OAM stat interrupt
          if (checkBit(get_stat(), 5)) {
            cpu.requestInterrupt(Interrupt::LCD_STAT);
          }
        }
      }
      break;
    case VBLANK:
      if (cyclesLeft >= VBLANK_CLOCKS) {
        cyclesLeft -= VBLANK_CLOCKS;

        // increment current line
        u8 scanline = get_ly() + 1;
        mmu.writeDirectly(LY, scanline);

        // check lyc interupt
        checkLYC(scanline);

        // reset scanline to 0 if > 153 (end of VBLANK)
        if (scanline >= 154) {

          // reset scanline to 0
          mmu.writeDirectly(LY, 0);
          
          mode = OAM;
          u8 stat = get_stat();
          stat = clearBit(stat, 0);
          stat = setBit(stat, 1);
          mmu.writeDirectly(STAT, stat);

          // OAM stat interrupt
          if (checkBit(get_stat(), 5)) {
            cpu.requestInterrupt(Interrupt::LCD_STAT);
          }
        }
      }
      break;
  }
}

void PPU::checkLYC(u8 scanline) {
  u8 lyc = get_lyc();
  u8 stat = get_stat();

  if (scanline == lyc) {
    stat = setBit(stat, 2);

    // LYC=LY stat interrupt
    if (checkBit(get_stat(), 6)) {
      cpu.requestInterrupt(Interrupt::LCD_STAT);
    }
  } else {
    stat = clearBit(stat, 2);
  }
  mmu.writeDirectly(STAT, stat);
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
      tileNum = (u8)mmu.readDirectly(tileAddress);
    } else {
      tileNum = (s8)mmu.readDirectly(tileAddress);
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
     u8 byte1 = mmu.readDirectly(tileLoc + line);
     u8 byte2 = mmu.readDirectly(tileLoc + line + 1);

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

    u8* pixelStartLocation = frameBuffer + (LCD_WIDTH * 3 * currentLine) + 3 * i;
    pixelStartLocation[0] = red;
    pixelStartLocation[1] = green;
    pixelStartLocation[2] = blue;
  }
}

int PPU::getcolor(int id, u16 palette_address) {
    u8 palette = mmu.readDirectly(palette_address);
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

 //return; // TODO: implement sprite rendering

  bool use8x16 = isObj8x16();
  int objSize = use8x16 ? 16 : 8;

  // can render up to 40 sprites, iterate through OAM table
  for (int i = 0; i < 40; i++) {
    u8 spriteIndex = i*4;
    u8 yPos = mmu.readDirectly(OAM_TABLE + spriteIndex) - 16;
    u8 xPos = mmu.readDirectly(OAM_TABLE + spriteIndex + 1) - 8;
    u8 tileIndex = mmu.readDirectly(OAM_TABLE + spriteIndex + 2);
    u8 attr = mmu.readDirectly(OAM_TABLE + spriteIndex + 3);

    // check sprite attributes
    //const bool bgOverObj = checkBit(attr, 7);
    const bool yFlip = checkBit(attr, 6);
    const bool xFlip = checkBit(attr, 5);
    const u16 pallete = checkBit(attr, 4) ? OBP1 : OBP0;

    // get current scanline
    u8 scanline = get_ly();

    // draw row of pixels in sprite if sprite intercepts scanline
    if (scanline >= yPos && (scanline < (yPos + objSize))) {
      int line = scanline - yPos;

      if (yFlip) {
        line -= objSize;
        line *= -1;
      }

      // look up tile data
      // 2 bytes of mem per line
      line *= 2;
      u16 sprite_addr = (0x8000 + (tileIndex * 16) + line);
      u8 byte1 = mmu.readDirectly(sprite_addr);
      u8 byte2 = mmu.readDirectly(sprite_addr + 1);

      for (int k = 7; k >= 0; k--) {
        int position = k;

        if (xFlip) {
          position -= 7;
          position *= -1;
        }

        u8 colorId = 0;
        if (checkBit(byte2, position)) {
          colorId = setBit(colorId, 1);
        }
        if (checkBit(byte1, position)) {
          colorId = setBit(colorId, 0);
        }

        int color = getcolor(colorId, pallete);

        if (color != 0) { // pixels with color index 0 (aka white) should be not rendered on sprites

          int xPixel = 0 - k;
          xPixel += 7;
          int pixel = xPos + xPixel;
  
          u8 red = 0;
          u8 green = 0;
          u8 blue = 0;
          switch(color) {
            case 0: red = 155; green = 188 ; blue = 15; break;   // WHITE = 0  #9bbc0f RGB: 155, 188, 15
            case 1:red = 139; green = 172 ; blue = 15; break;    // LIGHT_GRAY = 1 #8bac0f RGB: 139, 172, 15
            case 2: red = 48; green = 98 ; blue = 48; break;     // DARK_GRAY = 2  #306230 RGB: 48, 98, 48
            default: red = 15; green = 56; blue = 15; break;     // BLACK = 3  #0f380f RGB: 15, 56, 15
          }
        
          u8* pixelStartLocation = frameBuffer + (LCD_WIDTH * 3 * scanline) + 3 * pixel;
          pixelStartLocation[0] = red;
          pixelStartLocation[1] = green;
          pixelStartLocation[2] = blue;
        }
      }
    }
  }
}
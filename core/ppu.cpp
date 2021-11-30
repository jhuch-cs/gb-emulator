#include "./ppu.hpp"

PPU::PPU(PPU_Shared_Mem* PPU_Shared, CPU* cpu) : PPU_Shared(PPU_Shared), cpu(cpu) {
  mode = OAM;
  cyclesLeft = 0;
}

u8* PPU::getFrameBuffer() { return frameBuffer; }

// lcdc register helper functions 
// Bit 7	LCD and PPU enable	0=Off, 1=On
bool PPU::isLCDEnabled() { return checkBit(PPU_Shared->lcdc, 7); }
// Bit 6	Window tile map area	0=9800-9BFF, 1=9C00-9FFF
u16 PPU::windowTileMapArea() { return checkBit(PPU_Shared->lcdc, 6) ? 0x9C00 : 0x9800; }
// Bit 5	Window enable	0=Off, 1=On
bool PPU::isWindowEnabled() { return checkBit(PPU_Shared->lcdc, 5); }
// Bit 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
u16 PPU::tileDataArea() { return checkBit(PPU_Shared->lcdc, 4) ? 0x8000 : 0x8800; }
// Bit 3	BG tile map area	0=9800-9BFF, 1=9C00-9FFF
u16 PPU::bgTileMapArea() { return checkBit(PPU_Shared->lcdc, 3) ? 0x9C00 : 0x9800; }
// Bit 2	OBJ size	0=8x8, 1=8x16
bool PPU::isObj8x16() { return checkBit(PPU_Shared->lcdc, 2); }
// Bit 1	OBJ enable	0=Off, 1=On
bool PPU::isObjEnabled() { return checkBit(PPU_Shared->lcdc, 1); }
// Bit 0	BG and Window enable/priority	0=Off, 1=On
bool PPU::isBgWinEnabled() { return checkBit(PPU_Shared->lcdc, 0); }

void PPU::step(u8 cpuCyclesElapsed) {

  if (!isLCDEnabled()) { 
    mode = HBLANK;
    PPU_Shared->stat &= 0b00;
    PPU_Shared->ly = 0;
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
        PPU_Shared->stat |= 0b11;
      }
      break;
    case VRAM:
      if (cyclesLeft >= VRAM_CLOCKS) {
        cyclesLeft -= VRAM_CLOCKS;

        drawScanLine();
        
        mode = HBLANK;
        PPU_Shared->stat &= 0b00;

        // HBLANK stat interrupt
        if (checkBit(PPU_Shared->stat, 3)) {
          cpu->requestInterrupt(Interrupt::LCD_STAT);
        }
      }
      break;
    case HBLANK:
      if (cyclesLeft >= HBLANK_CLOCKS) {
        cyclesLeft -= HBLANK_CLOCKS;

        // get and increment scanline
        u8 scanline = ++PPU_Shared->ly;

        // check lyc interupt
        checkLYC();
        
        // check current scanline >= 144, then enter VBLANK, else enter OAM to prepare to draw another line
        if (scanline >= 144) {
          mode = VBLANK;
          cpu->requestInterrupt(VBLANK_INT); 
          PPU_Shared->stat = setBit(PPU_Shared->stat, 0);
          PPU_Shared->stat = clearBit(PPU_Shared->stat, 1);

          // VBLANK stat interrupt 
          if (checkBit(PPU_Shared->stat, 4)) {
            cpu->requestInterrupt(Interrupt::LCD_STAT);
          }
        } else {
          mode = OAM;
          PPU_Shared->stat = clearBit(PPU_Shared->stat, 0);
          PPU_Shared->stat = setBit(PPU_Shared->stat, 1);

          // OAM stat interrupt
          if (checkBit(PPU_Shared->stat, 5)) {
            cpu->requestInterrupt(Interrupt::LCD_STAT);
          }
        }
      }
      break;
    case VBLANK:
      if (cyclesLeft >= VBLANK_CLOCKS) {
        cyclesLeft -= VBLANK_CLOCKS;

        // increment current line
        u8 scanline = ++PPU_Shared->ly;

        // check lyc interupt
        checkLYC();

        // reset scanline to 0 if > 153 (end of VBLANK)
        if (scanline >= 154) {

          // reset scanline to 0
          PPU_Shared->ly = 0;
          
          mode = OAM;
          PPU_Shared->stat = clearBit(PPU_Shared->stat, 0);
          PPU_Shared->stat = setBit(PPU_Shared->stat, 1);

          // OAM stat interrupt
          if (checkBit(PPU_Shared->stat, 5)) {
            cpu->requestInterrupt(Interrupt::LCD_STAT);
          }
        }
      }
      break;
  }
}

void PPU::checkLYC() {
  if (PPU_Shared->ly == PPU_Shared->lyc) {
    PPU_Shared->stat = setBit(PPU_Shared->stat, 2);

    // LYC=LY stat interrupt
    if (checkBit(PPU_Shared->stat, 6)) {
      cpu->requestInterrupt(Interrupt::LCD_STAT);
    }
  } else {
    PPU_Shared->stat = clearBit(PPU_Shared->stat, 2);
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
  const u8 scrollY = PPU_Shared->scy;
  const u8 scrollX = PPU_Shared->scx;
  const u8 windowY = PPU_Shared->wy;
  const u8 windowX = PPU_Shared->wx - 7;

  // LCDC Bit 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
  u16 tileData = tileDataArea();
  bool unsig = checkBit(PPU_Shared->lcdc, 4);

  u8 currentLine = PPU_Shared->ly;

  // Check if window's Y position is within the current scanline and window is enabled
  // Tetris doesn't use a window, so this should be just set to the bgTileMapAreaa()
  u16 tileMap;
  bool winEnabled = (isWindowEnabled() && windowY <= currentLine);
  if (!winEnabled) {
    tileMap = bgTileMapArea();
  } else {
    tileMap = windowTileMapArea();
  }

  // calculate current row of tiles we are on
  u8 yPos = 0;
  if (!winEnabled) {
    yPos = scrollY + currentLine;
  } else {
    yPos = currentLine - windowY;
  }

  // calculate which row of pixels in the above tile we are on (32 tiles vertical, 8 pixels per tile)
  u16 currPixelRow = ((u8)(yPos/8)*32);

  u8* pixelStartOfRow = frameBuffer + (LCD_WIDTH * 3 * currentLine);

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
      tileNum = (u8)PPU_Shared->vram[tileAddress - VRAM_START];
    } else {
      tileNum = (s8)PPU_Shared->vram[tileAddress - VRAM_START];
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
    u8 byte1 = PPU_Shared->vram[tileLoc + line - VRAM_START];
    u8 byte2 = PPU_Shared->vram[tileLoc + line + 1 - VRAM_START];

    int colorBit = xPos % 8;
    colorBit -= 7;
    colorBit *= -1;

    int colorId = checkBit(byte2, colorBit);
    colorId <<= 1;
    colorId |= checkBit(byte1, colorBit);

    int color = getcolor(colorId, PPU_Shared->bgp);

    u8* pixelStartLocation = pixelStartOfRow + 3 * i;

    switch(color) {
      case 0:
        memcpy(pixelStartLocation, WHITE, 3);
        break;       
      case 1: 
        memcpy(pixelStartLocation, LIGHT_GRAY, 3);
        break;
      case 2: 
        memcpy(pixelStartLocation, DARK_GRAY, 3);
        break; 
      default:
        memcpy(pixelStartLocation, BLACK, 3);
        break; 
    }
  }
}

int PPU::getcolor(int id, u8 palette) {
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

  // get current scanline
  u8 scanline = PPU_Shared->ly;
  u8* pixelStartOfRow = frameBuffer + (LCD_WIDTH * 3 * scanline);

  // can render up to 40 sprites, iterate through OAM table
  for (int i = 0; i < 160; i += 4) {
    u8 yPos = PPU_Shared->oam[i] - 16;
    u8 xPos = PPU_Shared->oam[i + 1] - 8;
    u8 tileIndex = PPU_Shared->oam[i + 2];
    u8 attr = PPU_Shared->oam[i + 3];

    // check sprite attributes
    //const bool bgOverObj = checkBit(attr, 7);
    const bool yFlip = checkBit(attr, 6);
    const bool xFlip = checkBit(attr, 5);
    const u8 pallete = checkBit(attr, 4) ? PPU_Shared->obp1 : PPU_Shared->obp0;

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
      u16 sprite_addr = ((tileIndex * 16) + line);
      u8 byte1 = PPU_Shared->vram[sprite_addr];
      u8 byte2 = PPU_Shared->vram[sprite_addr + 1];

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

        if (colorId != 0) { // pixels with color index 0 (aka white) should be not rendered on sprites

          int xPixel = 7 - k;
          int pixel = xPos + xPixel;
        
          u8* pixelStartLocation = pixelStartOfRow + 3 * pixel;
          switch(color) {
            case 0:
              memcpy(pixelStartLocation, WHITE, 3);
              break;       
            case 1: 
              memcpy(pixelStartLocation, LIGHT_GRAY, 3);
              break;
            case 2: 
              memcpy(pixelStartLocation, DARK_GRAY, 3);
              break; 
            default:
              memcpy(pixelStartLocation, BLACK, 3);
              break; 
          }
        }
      }
    }
  }
}
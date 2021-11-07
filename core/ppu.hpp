#pragma once

#include "./mmu.hpp"
#include "./cpu.hpp"
#include "./util.hpp"

const u16 LCD_WIDTH = 160;
const u16 LCD_HEIGHT = 144;

const u16 OAM_CLOCKS = 80;
const u16 VRAM_CLOCKS = 172;
const u16 HBLANK_CLOCKS = 204;
const u16 VBLANK_CLOCKS = 4560;

const u16 OAM_TABLE = 0xFE00;

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

enum Mode {
  HBLANK,
  VBLANK,
  OAM,
  VRAM,
};

class PPU {
public:
  // Mode publicly accessable by MMU
  // During mode OAM: CPU cannot access OAM
  // During mode VRAM: CPU cannot access VRAM or OAM
  // During restricted modes, any attempt to read returns $FF, any attempt to write are ignored
  Mode mode;

  PPU(MMU& mmu, CPU& cpu);

  // Allow the PPU to cycle `cpuCyclesElapsed / 2` times per call
  void step(u8 cpuCyclesElapsed);

  // This is pulled out into a method, instead of public field access, so you only
  // have to update the buffer when SDL asks for it
  u8* getFrameBuffer();

  // Register getters
  u8 get_lcdc();
  u8 get_stat();
  u8 get_scy();
  u8 get_scx();
  u8 get_ly();
  u8 get_lyc();
  u8 get_dma();
  u8 get_bgp();
  u8 get_obp0();
  u8 get_obp1();
  u8 get_wy();
  u8 get_wx();

  // Register setters
  void set_ly(u8 ly);
  
  void checkLYC(u8 scanline);

  int getcolor(int id, u16 palette);
    
private:
  MMU mmu; 
  CPU cpu;

  unsigned int cyclesLeft;

  // 'lcdc' register helper functions
  bool isLCDEnabled();
  u16 windowTileMapArea();
  bool isWindowEnabled();
  u16 tileDataArea();
  u16 bgTileMapArea();
  bool isObj8x16();
  bool isObjEnabled();
  bool isBgWinEnabled();

  // Core functions
  void drawScanLine();
  void renderTiles();
  void renderSprites();

  // 160 x 144 x 3 (last dimenstion is pixel, rgb)
  static u8* frameBuffer;

  // 256 x 256
  u8* backgroundMap;
};
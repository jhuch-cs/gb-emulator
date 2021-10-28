#pragma once

#include "./cpu.hpp"
#include "./mmu.hpp"
#include "./util.hpp"

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

  PPU(CPU& cpu, MMU& mmu);

  // Allow the PPU to cycle `cpuCyclesElapsed / 2` times per call
  void step(u8 cpuCyclesElapsed);

  // This is pulled out into a method, instead of public field access, so you only
  // have to update the buffer when SDL asks for it
  u8* getFrameBuffer();

  CPU getCPU();
  MMU getMMU();

  // Register getters
  u16 get_lcdc();
  u16 get_stat();
  u16 get_scy();
  u16 get_scx();
  u16 get_ly();
  u16 get_lyc();
  u16 get_dma();
  u16 get_bgp();
  u16 get_obp0();
  u16 get_obp1();
  u16 get_wy();
  u16 get_wx();
    
private:
  CPU cpu;
  MMU mmu;

  unsigned int cyclesLeft;

  const int LCD_WIDTH = 144;
  const int LCD_HEIGHT = 160;

  const int OAM_CLOCKS = 80;
  const int VRAM_CLCOKS = 172;
  const int HBLANK_CLOCKS = 204;
  const int VBLANK_CLOCKS = 4560;

  const u16 OAM_TABLE = 0xFE00;

  // WHITE = 0  #9bbc0f RGB: 155, 188, 15
  // LIGHT_GRAY = 1 #8bac0f RGB: 139, 172, 15
  // DARK_GRAY = 2  #306230 RGB: 48, 98, 48
  // BLACK = 3  #0f380f RGB: 15, 56, 15

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
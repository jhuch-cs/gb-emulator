#pragma once

#include "./mmu.hpp"
#include "./cpu.hpp"
#include "./util.hpp"
#include "./sharedmemorystructs.hpp"

const u16 LCD_WIDTH = 160;
const u16 LCD_HEIGHT = 144;

const u16 OAM_CLOCKS = 80;
const u16 VRAM_CLOCKS = 172;
const u16 HBLANK_CLOCKS = 204;
const u16 VBLANK_CLOCKS = 456;

const u8 WHITE[] = {155, 188, 15};     // WHITE = 0  #9bbc0f RGB: 155, 188, 15
const u8 LIGHT_GRAY[] = {139, 172, 15}; // LIGHT_GRAY = 1 #8bac0f RGB: 139, 172, 15
const u8 DARK_GRAY[] = {48, 98, 48};   // DARK_GRAY = 2  #306230 RGB: 48, 98, 48
const u8 BLACK[] = {15, 56, 15};       // BLACK = 3  #0f380f RGB: 15, 56, 15

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

  PPU(PPU_Shared_Mem* PPU_Shared, CPU* cpu);

  // Allow the PPU to cycle `cpuCyclesElapsed / 2` times per call
  void step(u8 cpuCyclesElapsed);

  // This is pulled out into a method, instead of public field access, so you only
  // have to update the buffer when SDL asks for it
  u8* getFrameBuffer();
 
  void checkLYC();

  int getcolor(int id, u8 palette);
    
private:
  PPU_Shared_Mem* PPU_Shared;
  CPU* cpu;

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
  u8 frameBuffer[LCD_WIDTH * LCD_HEIGHT * 3] = {};
};
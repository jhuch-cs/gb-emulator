#pragma once

#include "./cpu.hpp"
#include "./mmu.hpp"
#include "./util.hpp"

enum Mode {
  OAM,
  VRAM,
  HBLANK,
  VBLANK,
};

class PPU {
public:
  PPU(CPU& cpu, MMU& mmu);

  // Allow the PPU to cycle `cpuCyclesElapsed / 2` times per call
  void step(u8 cpuCyclesElapsed);

  // This is pulled out into a method, instead of public field access, so you only
  // have to update the buffer when SDL asks for it
  u8* getFrameBuffer();

  // TODO: decide whether this logic should live in the PPU or if we should have the MMU handle 
  //       these by allowing it to access the current mode of the PPU
  // cpuReadVRAM()
  // cpuWriteVRAM()
  // cpuReadOAM()
  // cpuWriteOAM()  
    
private:
  CPU cpu;
  MMU mmu;
  Mode mode;

  // LCDC (0xFF40) - LCD Control
    // Bit 7	LCD and PPU enable	0=Off, 1=On
    // Bit 6	Window tile map area	0=9800-9BFF, 1=9C00-9FFF
    // Bit 5	Window enable	0=Off, 1=On
    // Bit 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
    // Bit 3	BG tile map area	0=9800-9BFF, 1=9C00-9FFF
    // Bit 2	OBJ size	0=8x8, 1=8x16
    // Bit 1	OBJ enable	0=Off, 1=On
    // Bit 0	BG and Window enable/priority	0=Off, 1=On
  const u8 lcdc = 0xFF40;

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
  const u8 stat = 0xFF41;

  // ScrollY (0xFF42) - y pos of background
  // ScrollX (0xFF43) - x pos of background
  const u8 scy = 0xFF42;
  const u8 scx = 0xFF43;

  // LY (0xFF44) - LCD Y Coordinate (aka current scanline)
  // Holds values 0-153, with 144-153 indicating VBLANK
  const u8 ly = 0xFF44;

  // LYC (0xFF45) - LY Compare
  const u8 lyc = 0xFF45;

  // DMA (0xFF46) - DMA Transfer and Start
  // 160 cycles
  const u8 dma = 0xFF46;

  // WHITE = 0  #9bbc0f RGB: 155, 188, 15
  // LIGHT_GRAY = 1 #8bac0f RGB: 139, 172, 15
  // DARK_GRAY = 2  #306230 RGB: 48, 98, 48
  // BLACK = 3  #0f380f RGB: 15, 56, 15

  // bgp (0xFF47) - BG palette data
    // Bit 7-6 Color for index 3
    // Bit 5-4 Color for index 2
    // Bit 3-2 Color for index 1
    // Bit 1-0 Color for index 0
  const u8 bgp = 0xFF47;

  // obp0 (0xFF48) - OBJ palette 0 data
  // Just like bgp but bits 1-0 ignored because color index 0 is transparent for sprites
  const u8 obp0 = 0xFF48;

  // obp1 (0XFF49) - OBJ palette 1 data
  const u8 obp1 = 0xFF49;

  // WindowY (0xFF4A) - y pos of window
  // WindowX (0xFF4B) - x pos - 7 of window
  const u8 wy = 0xFF4A;
  const u8 wx = 0xFF4B;

  void drawScanLine(u8 lcdCtrl);
  void renderTiles(u8 lcdCtrl);
  void renderSprites();

  // 160 x 144 x 3 (last dimenstion is pixel, rgb)
  static u8* frameBuffer;

  // 256 x 256
  u8* backgroundMap;

  // Helper Functions
  bool checkBit(u8 value, u8 index);
};
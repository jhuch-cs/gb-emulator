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

  // probably a lot of other public interface stuff that you'll discover in implementation
  // See https://gbdev.io/pandocs/Rendering.html
    
private:
  CPU cpu;
  MMU mmu;
  Mode mode;

  // 160 x 144 x 3 (last dimenstion is pixel, rgb)
  static u8* frameBuffer;

  // 256 x 256
  u8* backgroundMap;
};
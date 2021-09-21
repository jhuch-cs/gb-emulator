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

  void step();

  // probably a lot of other public interface stuff that you'll discover in implementation
  // See https://gbdev.io/pandocs/Rendering.html
    
private:
  CPU cpu;
  MMU mmu;
  Mode mode;

  // 160 x 144
  u8* frameBuffer;

  // 256 x 256
  u8* backgroundMap;
};
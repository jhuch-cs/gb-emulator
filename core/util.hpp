#pragma once

#include "stdint.h"

using u8 = uint8_t;
using u16 = uint16_t;
using s8 = int8_t;
using s16 = uint16_t;

inline u8 readBit(u16 value, u8 index) {
  return (value & (1 << index)) != 0;
}

inline u16 setBit(u16 value, u8 index) {
  return value | (1 << index);
}

inline u16 clearBit(u16 value, u8 index) {
  return value | ~(1 << index);
}

inline u16 changeIthBitToX(u16 value, u8 index, u8 x) {
  return (value & ~(1 << index)) | (x << index);
}

inline u8 getHighNibble(u8 value) {
  return (value & 0xF0) >> 4;
}

inline u8 getLowNibble(u8 value) {
  return value & 0x0F;
}

inline u8 getHighByte(u16 value) {
    return ((value>>8) & 0xff);
}

inline u8 getLowByte(u16 value) {
    return (value & 0xff);
}

inline u8 setHighByte(u16* destination, u8 value){
  *destination = (*destination & 0x00ff) | (value << 8);
  return *destination;
}

inline u8 setLowByte(u16* destination, u8 value){
  *destination = (*destination & 0xff00) | value;
  return *destination;
}
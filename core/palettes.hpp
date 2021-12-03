#pragma once

#include <stdio.h>
#include "./util.hpp"

struct Palette {
    u8 color_0[3]; //RGB
    u8 color_1[3];
    u8 color_2[3];
    u8 color_3[3];

    u8* operator[](int i) {
        switch (i) {
            case 0: return color_0;
            case 1: return color_1;
            case 2: return color_2;
            case 3: return color_3;
            default: return color_0;
        }
    }
};

class PaletteSwapper {
public:
    Palette& getNextPalette() {
        switch ((++swapIndex) % 15) {
            case 0:
                printf("Swapped to DMG\n"); 
                return dmg;
            case 1:
                printf("Swapped to Pokemon Blue\n");  
                return pokemon_blue;
            case 2: 
                printf("Swapped to Pokemon Red\n"); 
                return pokemon_red;
            case 3: 
                printf("Swapped to Kirokaze\n"); 
                return kirokaze;
            case 4: 
                printf("Swapped to IceCream\n"); 
                return ice_cream;
            case 5: 
                printf("Swapped to Mist\n"); 
                return mist;
            case 6: 
                printf("Swapped to Gray 2bit\n"); 
                return gray_2bit;
            case 7: 
                printf("Swapped to Demichrome 2bit\n"); 
                return demichrome_2bit;
            case 8: 
                printf("Swapped to Rustic\n"); 
                return rustic;
            case 9: 
                printf("Swapped to Wish\n"); 
                return wish;
            case 10: 
                printf("Swapped to Ayy4\n"); 
                return ayy4;
            case 11: 
                printf("Swapped to Crimson\n"); 
                return crimson;
            case 12: 
                printf("Swapped to Arq4\n"); 
                return arq4;
            case 13: 
                printf("Swapped to Pumpkin\n"); 
                return pumpkin;
            case 14: 
                printf("Swapped to Aqu4\n"); 
                return aqu4;
            default: 
                printf("Swapped to DMG\n"); 
                return dmg;
        }
    };

    Palette dmg =  {{0x9B, 0xBC, 0x0F},
                    {0x8B, 0xAC, 0x0F},
                    {0x30, 0x62, 0x30},
                    {0x0F, 0x38, 0x0F}};

    Palette pokemon_blue = {{0xFF, 0xFF, 0xFF},
                            {0x63, 0xA5, 0xFF},
                            {0x00, 0x00, 0xFF},
                            {0x00, 0x00, 0x00}};

    Palette pokemon_red = {{0xFF, 0xFF, 0xFF},
                           {0xFF, 0x84, 0x84},
                           {0x94, 0x3A, 0x3A},
                           {0x00, 0x00, 0x00}};

    // https://lospec.com/palette-list/kirokaze-gameboy
    Palette kirokaze = {{0xE2, 0xF3, 0xE4},
                        {0x94, 0xE3, 0x44},
                        {0x46, 0x87, 0x8F},
                        {0x33, 0x2C, 0x50}};

    // https://lospec.com/palette-list/ice-cream-gb
    Palette ice_cream = {{0xFF, 0xF6, 0xD3},
                         {0xF9, 0xA8, 0x75},
                         {0xEB, 0x6B, 0x6F},
                         {0x7C, 0x3F, 0x58}};

    // https://lospec.com/palette-list/mist-gb
    Palette mist = {{0xC4, 0xF0, 0xC2},
                    {0x5A, 0xB9, 0xA8},
                    {0x1E, 0x60, 0x6E},
                    {0x2D, 0x1B, 0x00}};

    // https://lospec.com/palette-list/2-bit-grayscale
    Palette gray_2bit = {{0xFF, 0xFF, 0xFF},
                         {0xB6, 0xB6, 0xB6},
                         {0x67, 0x67, 0x67},
                         {0x00, 0x00, 0x00}};

    // https://lospec.com/palette-list/2bit-demichrome
    Palette demichrome_2bit = {{0xE9, 0xEF, 0xEC},
                               {0xA0, 0xA0, 0x8B},
                               {0x55, 0x55, 0x68},
                               {0x21, 0x1E, 0x20}};

    // https://lospec.com/palette-list/rustic-gb
    Palette rustic = {{0xA9, 0x68, 0x68},
                      {0xED, 0xB4, 0xA1},
                      {0x76, 0x44, 0x62},
                      {0x2C, 0x21, 0x37}};

    // https://lospec.com/palette-list/wish-gb
    Palette wish =   {{0x8B, 0xE5, 0xFF},
                      {0x60, 0x8F, 0xCF},
                      {0x75, 0x50, 0xE8},
                      {0x62, 0x6E, 0x4C}};
    
    // https://lospec.com/palette-list/ayy4
    Palette ayy4 = {{0xF1, 0xF2, 0xDA},
                      {0xFF, 0xCE, 0x96},
                      {0xFF, 0x77, 0x77},
                      {0x00, 0x30, 0x3B}};

    // https://lospec.com/palette-list/crimson
    Palette crimson = {{0x1B, 0x03, 0x26},
                       {0x7A, 0x1C, 0x4B},
                       {0xBA, 0x50, 0x44},
                       {0xEF, 0xF9, 0xD6}};

    // https://lospec.com/palette-list/arq4
    Palette arq4 =   {{0x00, 0x00, 0x00},
                      {0x3A, 0x32, 0x77},
                      {0x67, 0x72, 0xA9},
                      {0xFF, 0xFF, 0xFF}};
    
    // https://lospec.com/palette-list/hallowpumpkin
    Palette pumpkin = {{0xF8, 0xF0, 0x88},
                       {0xF8, 0x90, 0x20},
                       {0x60, 0x28, 0x78},
                       {0x30, 0x00, 0x30}};

    // https://lospec.com/palette-list/blk-aqu4
    Palette aqu4 =   {{0x9F, 0xF4, 0xE5},
                      {0x00, 0xB9, 0xBE},
                      {0x00, 0x5F, 0x8C},
                      {0x00, 0x2B, 0x59}};
private: 
    int swapIndex = -1;
};
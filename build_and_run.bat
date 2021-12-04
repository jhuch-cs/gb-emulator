g++ -std=c++17 -O3 main.cpp .\core\*.cpp .\core\util.hpp -ISDL2\include -LSDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 -o gb-emulator
gb-emulator.exe gb_bios.bin Tetris.gb
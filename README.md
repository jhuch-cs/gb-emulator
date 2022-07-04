# gb-emulator

A Gameboy emulator implemented in C++ and with SDL2 for rendering. In addition to basic functionality, multiple cartridge types and palette switching are also implemented. Not implemented is audio and Gameboy Color functionality.

## What's What

* The source code for the emulator core is living in `./core`
* If you're developing on Windows, `build.bat` should compile the project to `gb-emulator.exe`, provided you have set up your SDL2 environment.
* If you're developing on a Unix-like machine (Linux, MacOS), `build.sh` should compile the project to an executable binary `gb-emulator`, provided you have the SDL2 dev environment installed. However, I haven't tested that, so YMMV.

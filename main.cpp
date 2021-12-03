#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

#include <SDL2/SDL.h>
#include "core/util.hpp"
#include "core/cartridge.hpp"
#include "core/mmu.hpp"
#include "core/cpu.hpp"
#include "core/ppu.hpp"
#include "core/timer.hpp"

const char TITLE[] = "gb-emulator";
const int WIDTH = 160;
const int HEIGHT = 144;
const int NUM_BYTES_OF_PIXELS = 3 * 144 * 160;
const double FPS = 60.0;


const int MAX_CYCLES_PER_FRAME = 69905;

u8 *boot_rom;
u8 *game_rom;
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

void free_boot_rom(void) {
	if (boot_rom != nullptr) {
		delete[] boot_rom;
		boot_rom = nullptr;
	}
}

void free_game_rom(void) {
	if (game_rom != nullptr) {
		delete[] game_rom;
		game_rom = nullptr;
	}
}

void destroy_window(void) {
	SDL_DestroyWindow(window);
}

void destroy_renderer(void) {
	SDL_DestroyRenderer(renderer);
}

void destroy_texture(void) {
	SDL_DestroyTexture(texture);
}

int load_binary_file(char *filename, u8 **buffer) {
	std::ifstream file;

	file.open(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		std::cerr << filename << ": " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	int file_size = file.tellg();

	*buffer = new u8[file_size];

	file.seekg(0);
	file.read((char *) *buffer, file_size);

	file.close();

	return file_size;
}

int main(int argc, char *argv[]) {
	//THIS SPOT BELOW IS ONLY IF PRELOADING THE FILES
	/*std::vector<char*> new_argv(argv, argv + argc);
	new_argv.push_back((char*)"./bootRom.gb");
	new_argv.push_back((char*)"./tetris.gb");
	new_argv.push_back(nullptr);
	argv = new_argv.data();
	argc = 3;*/
	std::cerr << "Starting up" << std::endl;
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " [boot_rom_file] [game_rom_file]" << std::endl;
		exit(EXIT_FAILURE);
	}

	char *boot_rom_filename = argv[1];
	char *game_rom_filename = argv[2];

	load_binary_file(boot_rom_filename, &boot_rom);

	atexit(free_boot_rom);

	load_binary_file(game_rom_filename, &game_rom);
	
	atexit(free_game_rom);

	/*if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL: " << SDL_Init(SDL_INIT_EVERYTHING) << std::endl;
		exit(EXIT_FAILURE);
	}*/


	if (SDL_Init(SDL_INIT_TIMER) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_TIMER: " << SDL_Init(SDL_INIT_TIMER) << std::endl;
		exit(EXIT_FAILURE);
	}
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_AUDIO: " << SDL_Init(SDL_INIT_AUDIO) << std::endl;
		exit(EXIT_FAILURE);
	}
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_GAMECONTROLLER: " << SDL_Init(SDL_INIT_GAMECONTROLLER) << std::endl;
		exit(EXIT_FAILURE);
	}
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_VIDEO: " << SDL_Init(SDL_INIT_VIDEO) << std::endl;
		exit(EXIT_FAILURE);
	}
	if (SDL_Init(SDL_INIT_JOYSTICK) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_JOYSTICK: " << SDL_Init(SDL_INIT_JOYSTICK) << std::endl;
		exit(EXIT_FAILURE);
	}
	/*if (SDL_Init(SDL_INIT_HAPTIC) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_HAPTIC: " << SDL_Init(SDL_INIT_HAPTIC) << std::endl;
		exit(EXIT_FAILURE);
	}*/
	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_EVENTS: " << SDL_Init(SDL_INIT_EVENTS) << std::endl;
		exit(EXIT_FAILURE);
	}
	/*if (SDL_Init(SDL_INIT_SENSOR) != 0) {
		//std::cout << "Error initializing SDL: " << SDL_GetError();
		std::cerr << "Error initializing SDL_INIT_SENSOR: " << SDL_Init(SDL_INIT_SENSOR) << std::endl;
		exit(EXIT_FAILURE);
	}*/

	std::cerr << "SDL has been initialized";

	std::atexit(SDL_Quit);

	int window_x = SDL_WINDOWPOS_CENTERED;
	int window_y = SDL_WINDOWPOS_CENTERED;
	int window_width = 4 * WIDTH;
	int window_height = 4 * HEIGHT;
	Uint32 window_flags = 0;

	window = SDL_CreateWindow(TITLE, window_x, window_y, window_width, window_height, window_flags);

	if (window == nullptr) {
		std::cout << "Error creating window: " << SDL_GetError();
		exit(EXIT_FAILURE);
	}

	std::atexit(destroy_window);

	int renderer_index = -1;
	Uint32 renderer_flags = 0;
	renderer = SDL_CreateRenderer(window, renderer_index, renderer_flags);

	if (renderer == nullptr) {
		std::cout << "Error creating renderer: " << SDL_GetError();
		exit(EXIT_FAILURE);
	}

	std::atexit(destroy_renderer);

	Uint32 texture_format = SDL_PIXELFORMAT_RGB24;
	int texture_access = SDL_TEXTUREACCESS_STATIC;
	texture = SDL_CreateTexture(renderer, texture_format, texture_access, WIDTH, HEIGHT);

	if (texture == nullptr) {
		std::cout << "Error creating texture: " << SDL_GetError();
		exit(EXIT_FAILURE);
	}

	std::atexit(destroy_texture);

	u8 pixels[NUM_BYTES_OF_PIXELS] = {};

	CartridgeInfo info = getInfo(game_rom);
	Cartridge* cartridge = createCartridge(game_rom, info);
	Input* input = new Input();
	MMU mmu = MMU(cartridge, input, boot_rom);
	CPU cpu = CPU(mmu);
	Timer timer = Timer(mmu, cpu);
	PPU ppu = PPU(mmu, cpu);

	bool quit = false;
	bool unlock_fps = false;
	while (!quit) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_W: {
							input->pressButton(UP);
						} break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A: {
							input->pressButton(LEFT);
						} break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S: {
							input->pressButton(DOWN);
						} break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D: {
							input->pressButton(RIGHT);
						} break;

						// B
						case SDL_SCANCODE_Z:
						case SDL_SCANCODE_J: {
							input->pressButton(B);
						} break;

						// A
						case SDL_SCANCODE_X:
						case SDL_SCANCODE_K: {
							input->pressButton(A);
						} break;

						// Select
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_G: {
							input->pressButton(SELECT);
						} break;

						// Start
						case SDL_SCANCODE_RETURN:
						case SDL_SCANCODE_H: {
							input->pressButton(START);
						} break;

						case SDL_SCANCODE_ESCAPE: {
							SDL_Event event = { .type = SDL_QUIT };
							SDL_PushEvent(&event);
						} break;

						case SDL_SCANCODE_SPACE: {
							unlock_fps = true;
						} break;

						default: {
						} break;
					}
				} break;

				case SDL_KEYUP: {
					switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_W: {
							input->unpressButton(UP);
						} break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A: {
							input->unpressButton(LEFT);
						} break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S: {
							input->unpressButton(DOWN);
						} break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D: {
							input->unpressButton(RIGHT);
						} break;

						// B
						case SDL_SCANCODE_Z:
						case SDL_SCANCODE_J: {
							input->unpressButton(B);
						} break;

						// A
						case SDL_SCANCODE_X:
						case SDL_SCANCODE_K: {
							input->unpressButton(A);
						} break;

						// Select
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_G: {
							input->unpressButton(SELECT);
						} break;

						// Start
						case SDL_SCANCODE_RETURN:
						case SDL_SCANCODE_H: {
							input->unpressButton(START);
						} break;

						case SDL_SCANCODE_SPACE: {
							unlock_fps = false;
						} break;
						
						default: {
						} break;
					}
				} break;

				case SDL_QUIT: {
					quit = true;
				} break;

				default: {
				};
			}
		}

		int cyclesThisFrame = 0;

		while (cyclesThisFrame < MAX_CYCLES_PER_FRAME) {
			int cycles = cpu.step();
			cyclesThisFrame += cycles;
			timer.step(cycles);
			ppu.step(cycles);
		}
		
		memcpy(pixels, ppu.getFrameBuffer(), NUM_BYTES_OF_PIXELS);

		SDL_RenderClear(renderer);
		SDL_UpdateTexture(texture, nullptr, pixels, WIDTH * sizeof(u8) * 3);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		if (!unlock_fps) {
			SDL_Delay(1000 / FPS);
		}
	}
	return 0;
}

#include <filesystem>
#include <fstream>
#include <iostream>

#include <SDL2/SDL.h>

#include "core/util.hpp"

const char TITLE[] = "gb-emulator";
const int WIDTH = 160;
const int HEIGHT = 144;
const double FPS = 60.0;

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
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " [boot_rom_file] [game_rom_file]" << std::endl;
		exit(EXIT_FAILURE);
	}

	char *boot_rom_filename = argv[1];
	char *game_rom_filename = argv[2];

	int boot_rom_size = load_binary_file(boot_rom_filename, &boot_rom);

	atexit(free_boot_rom);

	int game_rom_size = load_binary_file(game_rom_filename, &game_rom);

	atexit(free_game_rom);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "Error initializing SDL: " << SDL_GetError();
		exit(EXIT_FAILURE);
	}

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

	Uint32 texture_format = SDL_PIXELFORMAT_ARGB8888;
	int texture_access = SDL_TEXTUREACCESS_STATIC;
	texture = SDL_CreateTexture(renderer, texture_format, texture_access, WIDTH, HEIGHT);

	if (texture == nullptr) {
		std::cout << "Error creating texture: " << SDL_GetError();
		exit(EXIT_FAILURE);
	}

	std::atexit(destroy_texture);

	Uint32 pixels[WIDTH * HEIGHT] = {0};
	int pixel_index = 0; // TODO: Remove this when connecting the pixel display to the emulator core.

	bool quit = false;
	while (!quit) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_W: {
						} break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A: {
						} break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S: {
						} break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D: {
						} break;

						// B
						case SDL_SCANCODE_Z:
						case SDL_SCANCODE_J: {
						} break;

						// A
						case SDL_SCANCODE_X:
						case SDL_SCANCODE_K: {
						} break;

						// Select
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_G: {
						} break;

						// Start
						case SDL_SCANCODE_RETURN:
						case SDL_SCANCODE_H: {
						} break;

						case SDL_SCANCODE_ESCAPE: {
							SDL_Event event = { .type = SDL_QUIT };
							SDL_PushEvent(&event);
						} break;
					}
				} break;

				case SDL_KEYUP: {
					switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_W: {
						} break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A: {
						} break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S: {
						} break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D: {
						} break;

						// B
						case SDL_SCANCODE_Z:
						case SDL_SCANCODE_J: {
						} break;

						// A
						case SDL_SCANCODE_X:
						case SDL_SCANCODE_K: {
						} break;

						// Select
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_G: {
						} break;

						// Start
						case SDL_SCANCODE_RETURN:
						case SDL_SCANCODE_H: {
						} break;
					}
				} break;

				case SDL_QUIT: {
					quit = true;
				} break;
			}
		}

		// TODO: Remove these when connecting the pixel display to the emulator core.
		pixels[pixel_index] ^= 0x00FFFFFF;
		pixel_index = (pixel_index + 7) % (WIDTH * HEIGHT);

		SDL_RenderClear(renderer);
		SDL_UpdateTexture(texture, nullptr, pixels, WIDTH * sizeof(Uint32));
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		SDL_Delay(1000 / FPS);
	}
}

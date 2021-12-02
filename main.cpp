#include <fstream>
#include <iostream>
#include <cstring>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include "core/util.hpp"
#include "core/cartridge.hpp"
#include "core/gameboy.hpp"

const char TITLE[] = "gb-emulator";
const int WIDTH = 160;
const int HEIGHT = 144;
const int NUM_BYTES_OF_PIXELS = 3 * 144 * 160;
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

	int bootRomSize = load_binary_file(boot_rom_filename, &boot_rom);

	atexit(free_boot_rom);

	load_binary_file(game_rom_filename, &game_rom);
	
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
	Uint32 window_flags = SDL_WINDOW_RESIZABLE;

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

	Cartridge* cartridge = createCartridge(game_rom);
	GameBoy* gameBoy = new GameBoy(boot_rom, bootRomSize, cartridge);
	u8* frameBuffer = gameBoy->getFrameBuffer();

	SDL_SetWindowTitle(window, gameBoy->getTitle());

	SDL_GameController *gameController;
	SDL_GameControllerAddMappingsFromFile("gamecontroller.db");
	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i) && i == 0) {
			printf("Controller Detected: %s\n", SDL_GameControllerNameForIndex(i));
			gameController = SDL_GameControllerOpen(0);
		}
	}

	#ifdef FRAME_RATE
	u32 start = SDL_GetTicks();
	int frames = 0;
	#endif

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
							gameBoy->pressButton(UP);
						} break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A: {
							gameBoy->pressButton(LEFT);
						} break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S: {
							gameBoy->pressButton(DOWN);
						} break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D: {
							gameBoy->pressButton(RIGHT);
						} break;

						// B
						case SDL_SCANCODE_Z:
						case SDL_SCANCODE_J: {
							gameBoy->pressButton(B);
						} break;

						// A
						case SDL_SCANCODE_X:
						case SDL_SCANCODE_K: {
							gameBoy->pressButton(A);
						} break;

						// Select
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_G: {
							gameBoy->pressButton(SELECT);
						} break;

						// Start
						case SDL_SCANCODE_RETURN:
						case SDL_SCANCODE_H: {
							gameBoy->pressButton(START);
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
							gameBoy->unpressButton(UP);
						} break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A: {
							gameBoy->unpressButton(LEFT);
						} break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S: {
							gameBoy->unpressButton(DOWN);
						} break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D: {
							gameBoy->unpressButton(RIGHT);
						} break;

						// B
						case SDL_SCANCODE_Z:
						case SDL_SCANCODE_J: {
							gameBoy->unpressButton(B);
						} break;

						// A
						case SDL_SCANCODE_X:
						case SDL_SCANCODE_K: {
							gameBoy->unpressButton(A);
						} break;

						// Select
						case SDL_SCANCODE_RSHIFT:
						case SDL_SCANCODE_G: {
							gameBoy->unpressButton(SELECT);
						} break;

						// Start
						case SDL_SCANCODE_RETURN:
						case SDL_SCANCODE_H: {
							gameBoy->unpressButton(START);
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

				case SDL_CONTROLLERBUTTONDOWN:
					switch (event.cbutton.button) {
						case SDL_CONTROLLER_BUTTON_DPAD_UP: {
							gameBoy->pressButton(UP);
						} break;

						case SDL_CONTROLLER_BUTTON_DPAD_LEFT: {
							gameBoy->pressButton(LEFT);
						} break;

						case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
							gameBoy->pressButton(DOWN);
						} break;

						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: {
							gameBoy->pressButton(RIGHT);
						} break;

						// B
						case SDL_CONTROLLER_BUTTON_B: {
							gameBoy->pressButton(B);
						} break;

						// A
						case SDL_CONTROLLER_BUTTON_A: {
							gameBoy->pressButton(A);
						} break;

						// Select
						case SDL_CONTROLLER_BUTTON_GUIDE:
						case SDL_CONTROLLER_BUTTON_BACK: {
							gameBoy->pressButton(SELECT);
						} break;

						// Start
						case SDL_CONTROLLER_BUTTON_START: {
							gameBoy->pressButton(START);
						} break;

						case SDL_CONTROLLER_BUTTON_X: {
							unlock_fps = true;
						} break;

						case SDL_CONTROLLER_BUTTON_Y: {
							SDL_Event event = { .type = SDL_QUIT };
							SDL_PushEvent(&event);
						} break;

						default: {
							printf("Pressed: %i\n", event.cbutton.button);
						} break;
					} break;

				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button) {
						case SDL_CONTROLLER_BUTTON_DPAD_UP: {
							gameBoy->unpressButton(UP);
						} break;

						case SDL_CONTROLLER_BUTTON_DPAD_LEFT: {
							gameBoy->unpressButton(LEFT);
						} break;

						case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
							gameBoy->unpressButton(DOWN);
						} break;

						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: {
							gameBoy->unpressButton(RIGHT);
						} break;

						// B
						case SDL_CONTROLLER_BUTTON_B: {
							gameBoy->unpressButton(B);
						} break;

						// A
						case SDL_CONTROLLER_BUTTON_A: {
							gameBoy->unpressButton(A);
						} break;

						// Select
						case SDL_CONTROLLER_BUTTON_GUIDE:
						case SDL_CONTROLLER_BUTTON_BACK: {
							gameBoy->unpressButton(SELECT);
						} break;

						// Start
						case SDL_CONTROLLER_BUTTON_START: {
							gameBoy->unpressButton(START);
						} break;

						case SDL_CONTROLLER_BUTTON_X: {
							unlock_fps = false;
						} break;

						default: {
						} break;
					} break;

				default: {
				};
			}
		}

		#ifdef FRAME_RATE
		++frames;
		u32 elapsedMS = SDL_GetTicks() - start;

		if (elapsedMS) {
			double seconds = elapsedMS / 1000.0;
			double fps = frames / seconds;
			std::cout << fps << std::endl;
		}

		if (frames > 120) {
			start = SDL_GetTicks();
			frames = 0;
		}
		#endif

		gameBoy->step();

		SDL_RenderClear(renderer);
		SDL_UpdateTexture(texture, nullptr, frameBuffer, WIDTH * sizeof(u8) * 3);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		if (!unlock_fps) {
			SDL_Delay(1000 / FPS);
		}
	}
	return 0;
}

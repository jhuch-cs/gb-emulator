#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#define WIDTH 640
#define HEIGHT 480
#define SIZE 200
#define SPEED 600
#define GRAVITY 60
#define FPS 60
#define JUMP -1200

int main(int argc, char* argv[]) {
  /* Initializes the timer, audio, video, joystick,
  haptic, gamecontroller and events subsystems */
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("Error initializing SDL: %s\n", SDL_GetError());
    return 0;
  }

  /* Create a window */
  SDL_Window* window = SDL_CreateWindow("Hello Platformer!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
  
  if (!window) {
    printf("Error creating window: %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
  }

  /* Create a renderer */
  Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, render_flags);
  if (!renderer) {
    printf("Error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
  }

  // TODO: We're going to want a SDL_Surface, which is a raw pixel array
  // TODO: Then we're going to apply that surface to the SDL_Window's surface
  // window->surface = pixelSurface

  /*
    for (int i = 0; i < 144; i++) {
        for (int j = 0; j < 160; j++) {
            int red = pixels[j][i][0], blue = pixels[j][i][1], green = pixels[j][i][2];
            SDL_SetRenderDrawColor(screen->renderer, red, blue, green, 0xFF);
            SDL_RenderDrawPoint(screen->renderer, j, i);
        }
    }
    SDL_RenderPresent(screen->renderer);
  */

  /* Main loop */
  bool running = true, jump_pressed = false, can_jump = true, left_pressed = false, right_pressed = false;
  float x_pos = (WIDTH-SIZE)/2, y_pos = (HEIGHT-SIZE)/2, x_vel = 0, y_vel = 0;
  SDL_Rect rect = {(int) x_pos, (int) y_pos, SIZE, SIZE};
  SDL_Event event;
  while (running) {
    /* Process events */
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = false;
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_SPACE:
              jump_pressed = true;
              break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
              left_pressed = true;
              break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
              right_pressed = true;
              break;
            default:
              break;
            }
          break;
        case SDL_KEYUP:
          switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_SPACE:
              jump_pressed = false;
              break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
              left_pressed = false;
              break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
              right_pressed = false;
              break;
            default:
              break;
            }
          break;
        default:
          break;
      }
    }
    /* Clear screen */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    /* Move the rectangle */
    x_vel = (right_pressed - left_pressed) * SPEED;
    y_vel += GRAVITY;
    if (jump_pressed && can_jump) {
      can_jump = false;
      y_vel = JUMP;
    }
    x_pos += x_vel / 60;
    y_pos += y_vel / 60;
    if (x_pos <= 0)
      x_pos = 0;
    if (x_pos >= WIDTH - rect.w)
      x_pos = WIDTH - rect.w;
    if (y_pos <= 0)
      y_pos = 0;
    if (y_pos >= HEIGHT - rect.h) {
      y_vel = 0;
      y_pos = HEIGHT - rect.h;
      if (!jump_pressed)
        can_jump = true;
    }
    rect.x = (int) x_pos;
    rect.y = (int) y_pos;
    /* Draw the rectangle */
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    /* Draw to window and loop */
    SDL_RenderPresent(renderer);
    SDL_Delay(1000 / FPS);
  }

  /* Release resources */
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

#include <SDL.h>
#include <stdio.h>
#include "chip8.h"

#define ROM_PATH "./roms/IBM_Logo.ch8"

unsigned char sdl_key_to_chip8(SDL_Keycode key)
{
	switch (key)
	{
		case SDLK_x: return 0x0;
		case SDLK_1: return 0x1;
		case SDLK_2: return 0x2;
		case SDLK_3: return 0x3;
		case SDLK_q: return 0x4;
		case SDLK_w: return 0x5;
		case SDLK_e: return 0x6;
		case SDLK_a: return 0x7;
		case SDLK_s: return 0x8;
		case SDLK_d: return 0x9;
		case SDLK_z: return 0xA;
		case SDLK_c: return 0xB;
		case SDLK_4: return 0xC;
		case SDLK_r: return 0xD;
		case SDLK_f: return 0xE;
		case SDLK_v: return 0xF;
		default: return 0xFF;
	}
}

int main(int argc, char** argv)
{
	// variable
	char running = 1;
	SDL_Event event;
	CHIP8 chip8 = { 0 };
	unsigned char key_value;

	// initialize
	SDL_Init(SDL_INIT_VIDEO);
	CHIP8_Init(&chip8);
	CHIP8_loadROM(&chip8, ROM_PATH);

	SDL_Window* window = SDL_CreateWindow("Chip8 Emulator",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// main loop
	while (running)
	{
		// Process Event
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_KEYDOWN:
				key_value = sdl_key_to_chip8(event.key.keysym.sym);
				if (key_value != 0xFF) handle_key_event(&chip8, key_value, 1);
				break;
			case SDL_KEYUP:
				key_value = sdl_key_to_chip8(event.key.keysym.sym);
				if (key_value != 0xFF) handle_key_event(&chip8, key_value, 0);
				break;
			}
		}

		// logic
		CHIP8_emulateCycle(&chip8);

		Uint32 last_time = 0;
		Uint32 current_time = SDL_GetTicks();

		if (current_time - last_time >= 16)
		{
			if (chip8.delay_timer > 0) chip8.delay_timer--;
			if (chip8.sound_timer > 0) chip8.sound_timer--;
			last_time = current_time;
		}

		// render
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 64; x++)
			{
				if (chip8.display[y * 64 + x])
				{
					SDL_Rect rect = { x * 10,y * 10,10,10 };
					SDL_RenderFillRect(renderer, &rect);
				}
			}
		}

		SDL_RenderPresent(renderer);

		SDL_Delay(2);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}
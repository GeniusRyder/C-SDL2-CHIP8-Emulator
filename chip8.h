#pragma once
#include <SDL.h>

typedef enum
{
	STATE_RUNNING,
	STATE_WAITING_KEY
} Chip8_State;

typedef struct
{
	unsigned short opcode;
	unsigned char memory[4096];
	unsigned char V[16];
	unsigned short I;
	unsigned short pc;
	unsigned short stack[16];
	unsigned char sp;
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned char display[64 * 32];
	unsigned char keyboard[16];
	unsigned short font_base;
	Chip8_State state;
	unsigned char waiting_register;
} CHIP8;

void CHIP8_init(CHIP8* chip8);
void CHIP8_emulateCycle(CHIP8* chip8);
void CHIP8_loadROM(CHIP8* chip8, const char* filePath);
void handle_key_event(CHIP8* chip8, unsigned char key_value, char pressed);

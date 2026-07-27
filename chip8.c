#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

unsigned char font_data[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void CHIP8_Init(CHIP8* chip8)
{
	srand((unsigned)time(NULL));

	chip8->pc = 0x200;
	chip8->opcode = 0;
	chip8->I = 0;
	chip8->sp = 0;
	chip8->delay_timer = 0;
	chip8->sound_timer = 0;
	chip8->state = STATE_RUNNING;
	chip8->waiting_register = 0;
	chip8->font_base = 0x50;

	for (int i = 0; i < 16; i++)
	{
		chip8->stack[i] = 0;
		chip8->V[i] = 0;
		chip8->keyboard[i] = 0;
	}

	for (int i = 0; i < 64 * 32; i++) chip8->display[i] = 0;
	for (int i = 0; i < 4096; i++) chip8->memory[i] = 0;
	for (int i = 0; i < 80; i++) chip8->memory[chip8->font_base + i] = font_data[i];
}

void CHIP8_emulateCycle(CHIP8* chip8)
{
	if (chip8->state == STATE_WAITING_KEY) return;

	// Fetch Opcode
	chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];
	chip8->pc += 2;

	unsigned short nnn = chip8->opcode & 0x0FFF;
	unsigned char n = chip8->opcode & 0x000F;
	unsigned char x = (chip8->opcode & 0x0F00) >> 8;
	unsigned char y = (chip8->opcode & 0x00F0) >> 4;
	unsigned char kk = chip8->opcode & 0x00FF;

	// Decode Opcode
	unsigned char mode = (chip8->opcode & 0xF000) >> 12;
	switch (mode)
	{
		// Execute Opcode
	case 0x0:
		switch (chip8->opcode)
		{
			case 0x00E0:
				for (int i = 0; i < 64 * 32; i++) chip8->display[i] = 0;
				break;
			case 0x00EE:
				chip8->sp--;
				chip8->pc = chip8->stack[chip8->sp];
				break;
		}
		break;
	case 0x1:
		chip8->pc = nnn;
		break;
	case 0x2:
		chip8->stack[chip8->sp] = chip8->pc;
		chip8->sp++;
		chip8->pc = nnn;
		break;
	case 0x3:
		if (chip8->V[x] == kk) chip8->pc += 2;
		break;
	case 0x4:
		if (chip8->V[x] != kk) chip8->pc += 2;
		break;
	case 0x5:
		switch (chip8->opcode & 0x000F)
		{
		case 0:
			if (chip8->V[x] == chip8->V[y]) chip8->pc += 2;
			break;
		}
		break;
	case 0x6:
		chip8->V[x] = kk;
		break;
	case 0x7:
		chip8->V[x] += kk;
		break;
	case 0x8:
		switch (chip8->opcode & 0x000F)
		{
		case 0x0:
			chip8->V[x] = chip8->V[y];
			break;
		case 0x1:
			chip8->V[x] = chip8->V[x] | chip8->V[y];
			break;
		case 0x2:
			chip8->V[x] = chip8->V[x] & chip8->V[y];
			break;
		case 0x3:
			chip8->V[x] = chip8->V[x] ^ chip8->V[y];
			break;
		case 0x4:
		{
			unsigned short sum = chip8->V[x] + chip8->V[y];
			chip8->V[0xF] = sum > 255 ? 1 : 0;
			chip8->V[x] = chip8->V[x] + chip8->V[y];
		}
			break;
		case 0x5:
		{
			unsigned short result = chip8->V[x] - chip8->V[y];
			chip8->V[0xF] = chip8->V[x] >= chip8->V[y] ? 1 : 0;
			chip8->V[x] = chip8->V[x] - chip8->V[y];
		}
			break;
		case 0x6:
			chip8->V[0xF] = chip8->V[x] & 1;
			chip8->V[x] = chip8->V[x] >> 1;
			break;
		case 0x7:
		{
			unsigned short result = chip8->V[y] - chip8->V[x];
			chip8->V[0xF] = chip8->V[y] >= chip8->V[x] ? 1 : 0;
			chip8->V[x] = chip8->V[y] - chip8->V[x];
			break;
		}
		case 0xE:
			chip8->V[0xF] = chip8->V[x] >> 7;
			chip8->V[x] = chip8->V[x] << 1;
			break;
		}
		break;
	case 0x9:
		switch (chip8->opcode & 0x000F)
		{
		case 0:
			if (chip8->V[x] != chip8->V[y]) chip8->pc += 2;
			break;
		}
		break;
	case 0xA:
		chip8->I = nnn;
		break;
	case 0xB:
		chip8->pc = chip8->V[0] + nnn;
		break;
	case 0xC:
		chip8->V[x] = (rand() % 256) & kk;
		break;
	case 0xD:
		chip8->V[0xF] = 0;
		for (int row = 0; row < n; row++)
		{
			unsigned char sprite_byte = chip8->memory[chip8->I + row];
			for (int col = 0; col < 8; col++)
			{
				if (sprite_byte & (0x80 >> col))
				{
					int px = (chip8->V[x] + col) % 64;
					int py = (chip8->V[y] + row) % 32;
					if (chip8->display[py * 64 + px]) chip8->V[0xF] = 1;
					chip8->display[py * 64 + px] ^= 1;
				}
			}
		}
		break;
	case 0xE:
		switch (chip8->opcode & 0x00FF)
		{
		case 0x009E:
			if (chip8->keyboard[chip8->V[x]] == 1) chip8->pc += 2;
			break;
		case 0x00A1:
			if (chip8->keyboard[chip8->V[x]] == 0) chip8->pc += 2;
			break;
		}
		break;
	case 0xF:
		switch (chip8->opcode & 0x00FF)
		{
		case 0x0007:
			chip8->V[x] = chip8->delay_timer;
			break;
		case 0x000A:
			chip8->state = STATE_WAITING_KEY;
			chip8->waiting_register = x;
			chip8->pc -= 2;
			return;
			break;
		case 0x0015:
			chip8->delay_timer = chip8->V[x];
			break;
		case 0x0018:
			chip8->sound_timer = chip8->V[x];
			break;
		case 0x001E:
			chip8->I += chip8->V[x];
			break;
		case 0x0029:
			chip8->I = chip8->font_base + chip8->V[x] * 5;
			break;
		case 0x0033:
			chip8->memory[chip8->I] = chip8->V[x] / 100;
			chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
			chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
			break;
		case 0x0055:
			for (int i = 0; i <= x; i++) chip8->memory[chip8->I + i] = chip8->V[i];
			break;
		case 0x0065:
			for (int i = 0; i <= x; i++) chip8->V[i] = chip8->memory[chip8->I + i];
			break;
		}
		break;
	}
}

void CHIP8_loadROM(CHIP8* chip8, const char* filePath)
{
	for (int i = 0x200; i < 4096; i++) chip8->memory[i] = 0;

	FILE* file = fopen(filePath, "rb");
	fread(&chip8->memory[0x200], 1, 4096 - 0x200, file);
	fclose(file);
}

void handle_key_event(CHIP8* chip8, unsigned char key_value, char pressed)
{
	if (pressed)
	{
		chip8->keyboard[key_value] = 1;
		if (chip8->state == STATE_WAITING_KEY)
		{
			chip8->V[chip8->waiting_register] = key_value;
			chip8->state = STATE_RUNNING;
			chip8->pc += 2;
		}
	}
	else chip8->keyboard[key_value] = 0;
}
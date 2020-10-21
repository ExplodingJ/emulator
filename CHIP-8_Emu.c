#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
void DisassembleChip8Op(uint8_t *codebuffer, int pc)
{
	uint8_t *code = &codebuffer[pc];
	uint8_t firstnib = (code[0] >> 4);
	printf("%04x %02x %02x ", pc, code[0], code[1]);
	switch (firstnib)
	{
		case 0x0:
			switch (code[1])
			{
				case 0xe0: printf("%-10s", "CLS"); break;
				case 0xee: printf("%-10s", "RTS"); break;
				default: printf("UNKNOWN 0"); break;
			}
			break;
		case 0x1: printf("%-10s $%01x%02x", "JUMP", code[0]&0xf, code[1]); break;
		case 0x2: printf("%-10s $%01x%02x", "CALL", code[0]&0xf, code[1]); break;
		case 0x3: printf("%-10s V%01X,#$%02x", "SKIP.EQ", code[0]&0xf, code[1]); break;
		case 0x4: printf("%-10s V%01X,#$%02x", "SKIP.NE", code[0]&0xf, code[1]); break;
		case 0x5: printf("%-10s V%01X,V%01X", "SKIP.EQ", code[0]&0xf, code[1]>>4); break;
		case 0x6: printf("%-10s V%01X,#$%02x", "MVI", code[0]&0xf, code[1]); break;
		case 0x7: printf("%-10s V%01X,#$%02x", "ADI", code[0]&0xf, code[1]); break;
		case 0x8:
			{
				uint8_t lastnib = code[1]>>4;
				switch(lastnib)
				{
					case 0: printf("%-10s V%01X,V%01X", "MOV.", code[0]&0xf, code[1]>>4); break;
					case 1: printf("%-10s V%01X,V%01X", "OR.", code[0]&0xf, code[1]>>4); break;
					case 2: printf("%-10s V%01X,V%01X", "AND.", code[0]&0xf, code[1]>>4); break;
					case 3: printf("%-10s V%01X,V%01X", "XOR.", code[0]&0xf, code[1]>>4); break;
					case 4: printf("%-10s V%01X,V%01X", "ADD.", code[0]&0xf, code[1]>>4); break;
					case 5: printf("%-10s V%01X,V%01X,V%01X", "SUB.", code[0]&0xf, code[0]&0xf, code[1]>>4); break;
					case 6: printf("%-10s V%01X,V%01X", "SHR.", code[0]&0xf, code[1]>>4); break;
					case 7: printf("%-10s V%01X,V%01X,V%01X", "SUB.", code[0]&0xf, code[1]>>4, code[1]>>4); break;
					case 0xe: printf("%-10s V%01X,V%01X", "SHL.", code[0]&0xf, code[1]>>4); break;
					default: printf("UNKNOWN 8"); break;
				}
			}
			break;
		case 0x9: printf("%-10s V%01X,V%01X", "SKIP.NE", code[0]&0xf, code[1]>>4); break;
		case 0xa: printf("%-10s I,#$%01x%02x", "MVI", code[0]&0xf, code[1]); break;
		case 0xb: printf("%-10s $%01x%02x(V0)", "JUMP", code[0]&0xf, code[1]); break;
		case 0xc: printf("%-10s V%01X,#$%02X", "RNDMSK"); break;
		case 0xd: printf("%-10s V%01X,V%01X,#$%01x", "SPRITE", code[0]&0xf, code[1]>>4, code[1]&0xf); break;
		case 0xe:
			switch(code[1])
			{
				case 0x9E: printf("%-10s V%01X", "SKIPKEY.Y", code[0]&0xf); break;
				case 0xA1: printf("%-10s V%01X", "SKIPKEY.N", code[0]&0xf); break;
				default: printf("UNKNOWN E"); break;
			}
			break;
		case 0xf:
			switch(code[1])
			{
				case 0x07: printf("%-10s V%01X,DELAY", "MOV", code[0]&0xf); break;
				case 0x0a: printf("%-10s V%01X", "KEY", code[0&0xf]); break;
				case 0x15: printf("%-10s DELAY,V%01X", "MOV", code[0]&0xf); break;
				case 0x18: printf("%-10s SOUND,V%01X", "MOV", code[0]&0xf); break;
				case 0x1e: printf("%-10s I,V%01X", "ADI", code[0]&0xf); break;
				case 0x29: printf("%-10s I,V%01X", "SPRITECHAR", code[0]&0xf); break;
				case 0x33: printf("%-10s (I),V%01X", "MOVBCD", code[0]&0xf); break;
				case 0x55: printf("%-10s (I),V0-V%01X", "MOVM", code[0]&0xf); break;
				case 0x65: printf("%-10s V0-V%01X,(I)", "MOVM", code[0]&0xf); break;
				default: printf("UNKNOWN F"); break;
			}
			break;
	}
}
typedef struct Chip8State {
	uint8_t V[16];
	uint16_t I;
	uint16_t SP;
	uint16_t PC;
	uint8_t delay;
	uint8_t sound;
	uint8_t *memory;
	uint8_t *screen;
} Chip8State;
#define FONT_BASE 0
#define FONT_SIZE 5*16
Chip8State* InitChip8(void)
{
	Chip8State* s = calloc(sizeof(Chip8State), 1);
	s->memory = calloc(1024*4, 1);
	s->screen = &s->memory[0xf00];
	s->SP = 0xfa0;
	s->PC = 0x200;
	memcpy(&s->memory[FONT_BASE], font4x5, FONT_SIZE)
	return s;
}
void EmulateChip8Op(Chip8State *state)
{
	uint8_t *op = &state->memory[state->PC];
	int highnib = (*op & 0xf0) >> 4;
	switch (highnib)
	{
		case 0x00: UnimplementedInstruction(state); break;
		case 0x01:
			{
				uint16_t target = ((code[0]&0xf)<<8) | code[1];
				state->PC = target;
			}
			break;
		case 0x02: UnimplementedInstruction(state); break;
		case 0x03:
			{
				uint8_t reg = code[0] & 0xf;
				if (state->V[reg] == code[1])
					state->PC+=2;
				state->PC+=2;
			}
			break;
		case 0x04: UnimplementedInstruction(state); break;
		case 0x05: UnimplementedInstruction(state); break;
		case 0x06:
			{
				uint8_t reg = code[0] & 0xf;
				state->V[reg] = code[1];
				state->PC+=2;
			}
			break;
		case 0x07: UnimplementedInstruction(state); break;
		case 0x08: UnimplementedInstruction(state); break;
		case 0x09: UnimplementedInstruction(state); break;
		case 0x0a:
			{
				state->I = ((code[0] & 0xf)<<8) | code [1];
				state->PC+=2;
			}
			break;
		case 0x0b: UnimplementedInstruction(state); break;
		case 0x0c: UnimplementedInstruction(state); break;
		case 0x0d: UnimplementedInstruction(state); break;
		case 0x0e: UnimplementedInstruction(state); break;
		case 0x0f: UnimplementedInstruction(state); break;
	}
}
static void OpF(Chip8State *state, uint8_t *code)
{
	int reg = code[0]&0xf;
	switch (code[1])
	{
		case 0x07: state->V[reg] = state->delay; break;
		case 0x15: state->delay = state->V[reg]; break;
		case 0x18: state->sound = state->V[reg]; break;
	}
}
static void OpE(Chip8State *state, uint8_t *code)
{
	int reg = code[0]&0xf;
	switch (code[1])
	{
		case 0x9e:
		if (state->key_state[state->V[reg]] != 0)
			state->PC+=2;
		break;
		case 0xa1:
		if (state->key_state[state->V[reg]] == 0)
			state->PC+=2;
		break;
		default:
			UnimplementedInstruction(state);
			break;
	}
	state->PC+=2;
}
static void OpFX29(Chip8State *state, uint8_t *code)
{
	int reg = code[0]&0xf;
	state->I = FONT_BASE+(state->V[reg] * 5);
}
int main(int argc, char**argv)
{
	FILE *f = fopen(argv[1], "rb");
	if (f==NULL)
	{
		printf("ERROR: Couldn't open %s\n", argv[1]);
		exit(1);
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);
	unsigned char *buffer=malloc(fsize+0x200);
	fread(buffer+0x200, fsize, 1, f);
	fclose(f);
	int pc = 0x200;
	while (pc < (fsize+0x200))
	{
		DisassembleChip8Op(buffer, pc);
		pc += 2;
		printf("\n");
	}
	return 0;
}
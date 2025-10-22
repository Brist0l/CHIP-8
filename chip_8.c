#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>

#include"debug.h"
#include"display.h"

bool debug_flag = true;

// CHIP-8 has 16 8-bit registers (V0 - VF)  
typedef struct _registers{ 
	char V[0xF];
	unsigned _BitInt(12) I; // Address register 
	unsigned _BitInt(12) PC; // Program counter register 
}_registers;

// 48 bytes worth of stack
unsigned char stack[48]; 
/* 4096 bytes worth of memory . first 512(0x200) bytes are reserved i.e. the opcodes need to be loaded
from 0x200.*/
unsigned char memory[0x1000]; 

// Both the timers need to be decremented by 1 , 60 times per sec (i.e. 60 Hz)
unsigned char delay_timer;
unsigned char sound_timer; // It makes the computer "beep" as long as it's above 0

struct Game *g = NULL;

void _fontset();
const unsigned short fetch(_registers *registers);
void game_run(struct Game *g,_registers *registers);

/* Font:
It ranged from 0-F and it was stored in the reserved memory (anywhere in it is fine but conventionally it
was stored in 0x050–0x09F[i.e. 80 bytes , there are 16 symbols with 5 bytes each]).

eg:

"0":
****
*  *
*  *
*  *
****

bin:
11110000
10010000
10010000
10010000
11110000

Hex:
0xF0
0x90
0x90
0x90
0xF0
*/

void _fontset(){
	logmsg("_fontset",true,debug_flag);

	unsigned char fonts[16][5] = { 
		{0xF0, 0x90, 0x90, 0x90, 0xF0}, // 0
		{0x20, 0x60, 0x20, 0x20, 0x70}, // 1
		{0xF0, 0x10, 0xF0, 0x80, 0xF0}, // 2
		{0xF0, 0x10, 0xF0, 0x10, 0xF0}, // 3
		{0x90, 0x90, 0xF0, 0x10, 0x10}, // 4
		{0xF0, 0x80, 0xF0, 0x10, 0xF0}, // 5
		{0xF0, 0x80, 0xF0, 0x90, 0xF0}, // 6
		{0xF0, 0x10, 0x20, 0x40, 0x40}, // 7
		{0xF0, 0x90, 0xF0, 0x90, 0xF0}, // 8
		{0xF0, 0x90, 0xF0, 0x10, 0xF0}, // 9
		{0xF0, 0x90, 0xF0, 0x90, 0x90}, // A
		{0xE0, 0x90, 0xE0, 0x90, 0xE0}, // B
		{0xF0, 0x80, 0x80, 0x80, 0xF0}, // C
		{0xE0, 0x90, 0x90, 0x90, 0xE0}, // D
		{0xF0, 0x80, 0xF0, 0x80, 0xF0}, // E
		{0xF0, 0x80, 0xF0, 0x80, 0x80}  // F
	};

	int pos = 0x050;
	

	for(int i = 0; i < 16; i++){
		for(int j = 0;j < 5;j++){
			if(debug_flag){
				printf("val is: %02x at 0x%02x\n",fonts[i][j],pos);
			}
			memset(&memory[pos++],fonts[i][j],sizeof(fonts[i][j]));
		}
	}

	logmsg("_fontset",false,debug_flag);
}

const unsigned short fetch(_registers *registers){
	logmsg("fetch",true,debug_flag);
	//_fillopcode();

	unsigned short opcode = 0x0;
	unsigned short MSB = memory[registers->PC];
	MSB <<= 8; // shifting the number into MSB side 
	unsigned short LSB = memory[registers->PC + 1];
	opcode = opcode | MSB | LSB; 

	registers->PC += 2; // Increment PC by 2 cuz 2 consequitive bytes of memory has been accessed	

	if(debug_flag){
		printf("The opcode is:%04X(dec equivalent:%06d and bin is %016b)\n",opcode,opcode,opcode);
		printf("The PC value after incrementing is now:%x\n",(int)registers->PC);
	}

	logmsg("fetch",false,debug_flag);

	return opcode;
}

/* The opcode's nibbles(4 bits) have different meaning. The first nibble tells what category of 
 * instruction is it.
 *
 * The 2'nd Nibble is `X` which tells which register needs to be used (where `X` is the number. i.e.
 * Vx)
 * The 3'rd Nibble is `Y` which tells which is the 2nd register to be used(where `Y` is the number. i.e.
 * Vy)
 * Note that both `X` and `Y` can range from 0-F.
 * The 4'th Nibble is `N` and it's a const number 
 * 
 * `NN` The 2nd byte (i.e. 3rd and 4th nibble) is a const number
 * `NNN` The 2nd,3rd and 4th nibbles are an address (i.e. a 12 bit memory address)
 */
void execute(const unsigned short opcode,_registers *registers){
	logmsg("execute",true,debug_flag);
	unsigned short first_nibble = 0xF000;
	unsigned short second_nibble = 0x0F00;
	unsigned short third_nibble = 0x00F0;
	unsigned short fourth_nibble = 0x000F;
	unsigned short N;
	unsigned short NN = 0x00FF;
	unsigned short NNN = 0x0FFF;
	unsigned short X; 
	unsigned short Y;

	if(debug_flag){
		printf("First nibble before: %016b(%X)\n",first_nibble,first_nibble);
		printf("Second nibble before: %016b(%X)\n",second_nibble,second_nibble);
		printf("Third nibble before: %016b(%X)\n",third_nibble,third_nibble);
		printf("Fourth nibble before: %016b(%X)\n",fourth_nibble,fourth_nibble);
	}

	/* Basically this gets the first nibble because the starting 4 bits are 1s and it will turn 
	 * itself into the first 4 bits of the opcode by ANDing it together and the rest are 0s so 
	 * whatever be the case they will always remain 0.
	 */
	first_nibble &= opcode; 
	first_nibble >>= 12;
	second_nibble &= opcode; 
	second_nibble >>= 8;
	third_nibble &= opcode; 
	third_nibble >>= 4;
	fourth_nibble &= opcode; 
	N = fourth_nibble;
	NN &= opcode;
	NNN &= opcode;
	X = second_nibble;
	Y = third_nibble;

	if(debug_flag){
		printf("First nibble after: %016b(%X)\n",first_nibble,first_nibble);
		printf("Second nibble after: %016b(%X)\n",second_nibble,second_nibble);
		printf("Third nibble after: %016b(%X)\n",third_nibble,third_nibble);
		printf("Fourth nibble after: %016b(%X)\n",fourth_nibble,fourth_nibble);
	}

	printf("The addr of g is %p and is_runing %d\n",g,g->is_running);

	switch(first_nibble){
		case 0x0:
			/* Valid instructions:
			 * 00E0 => Clears the screen
			 * 00EE => Returns from the subroutine*/

			printf("Welcome to case 0\n");
			switch(fourth_nibble){
				case 0:
					printf("Instruction: 00E0 => Clears the screen\n");
					clear_screen(g);
					printf("Clearing screen\n");
					break;
				case 0xE:
					printf("Instruction: 00EE => Returns from the subroutine\n");
					printf("returing\n");
					break;
				default:
					printf("Bad instruction\n");
			}
			
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0x1:
			/* Valid instructions:
			 * 1NNN => Jumps to address NNN */

			printf("Welcome to case 1\n");
			registers->PC = NNN;
			if(debug_flag)
				printf("PC's value is 0x%X\n",(int)registers->PC);

			break;
		case 0x2:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0x3:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0x4:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0x5:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0x6:
			/* Valid instructions:
			 * 6XNN : Sets Vx to NN */
			printf("Welcome to case 6\n");

			registers->V[X] = NN;

			if(debug_flag)
				printf("Register %d's value is 0x%X\n",X,registers->V[X]);

			break;
		case 0x7:
			/* Valid instructions:
			 * 7XNN : Adds Vx to NN */
			printf("Welcome to case 7\n");

			registers->V[X] += NN;

			if(debug_flag)
				printf("Register value is 0x%X\n",registers->V[X]);

			break;
		case 0x8:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0x9:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xA:
			/* Valid instructions:
			 * ANNN : Sets I to the address NNN.*/

			if(debug_flag){
				printf("Welcome to case A\n");
				printf("Register value is 0x%X\n",(int)registers->I);
				printf("mem at the location is:%X\n",memory[registers->I]);
			}

			registers->I = NNN;

			break;
		case 0xB:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xC:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xD:
			/* Valid instructions:
			 * DXYN : Draw at coordinate (Vx,Vy) .*/

			if(debug_flag){
				printf("Welcome to case D\n");
				printf("memory[I] has : %X\n",memory[(int)registers->I]);
				printf("Height is %d\n",N);
				for(int i = 0; i < N;i++)
					printf("=>%08b\n",memory[(registers->I) +i]);
			}

			int x_coor = registers->V[X];
			int y_coor = registers->V[Y];

			for(int i = 0; i < N;i++)
				draw(g,x_coor,y_coor + i,memory[(registers->I) + i]);

			break;
		case 0xE:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xF:
			printf("First nibble after: %016b\n",first_nibble);
			break;

		default:
			printf("First nibble after: %016b\n",first_nibble);
			break;
	}

	logmsg("execute",false,debug_flag);
}

void game_run(struct Game *g , _registers *registers){
	while(g->is_running){
		game_events(g);
		//game_draw(g);
		execute(fetch(registers),registers);
		if(registers-> PC == 0x333)
			g->is_running = false;
		SDL_Delay(1000); // i.e. 60Hz
	}
}

void display_ROM(FILE* rom){
	int data;
	int j = 0;
	printf("The Instructions of the .ch8 file are:\n");
	
	while((data = fgetc(rom)) != EOF){
		printf("%02X",(unsigned char)data);
		if(++j == 2){
			printf("\n");
			j= 0;
		}
	}

	fseek(rom,0,SEEK_SET);
	printf("Seeked to %ld\n",ftell(rom));
}

bool load_ROM(const char *name){
	FILE* rom;
	int data;
	unsigned int i = 0x200;

	rom = fopen(name,"rb");
	if(rom == NULL){
		fprintf(stderr,"Couldn't read %s\n",name);
		return false;
	}
	
	display_ROM(rom);

	while((data = fgetc(rom)) != EOF)
		memory[i++] = (unsigned char) data;

	fclose(rom);
	return true;
}

int main(){
	_registers registers;
	registers.PC = 0x200; // starting from the unreserved section
	_fontset();
	_memoryframe(0x050,0x200);	
	//printf("%d\n",EXIT_FAILURE);
	bool exit_status = EXIT_FAILURE;

	//printf("game: %p\n",g);
	
	if(!(load_ROM("ROMs/IBM_Logo.ch8")))
		return -1;

	_memoryframe(0x200,0x300);

	if(game_new(&g)){
		printf("game: %p\n",g);
		game_run(g,&registers);
		exit_status = EXIT_SUCCESS;
	}

	game_free(&g);
	//printf("game: %p\n",g);
	//printf("%d\n",EXIT_SUCCESS);

	return exit_status;
}

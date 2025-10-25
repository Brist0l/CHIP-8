#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>

#include"debug.h"
#include"display.h"

bool debug_flag = true;

// CHIP-8 has 16 8-bit registers (V0 - VF)  
// Registers are just "registers" , there are no "signed" or "unsigned" registers. The signed/unsigned
// is a software level consturct. All registers are "technically" unsigned because they just hold the 
// data and don't dictate what's the orientation of the data(i.e. if it's 2's compliment or not).
typedef struct _registers{ 
	uint8_t V[0xF + 1]; // A total of 16 registers (15 +1)
	unsigned _BitInt(12) I; // Address register 
	unsigned _BitInt(12) PC; // Program counter register 
}_registers;

// 16 2-bytes worth of stack
unsigned short stack[16]; 
/* 4096 bytes worth of memory . first 512(0x200) bytes are reserved i.e. the opcodes need to be loaded
from 0x200.*/
uint8_t memory[0x1000]; 

// Both the timers need to be decremented by 1 , 60 times per sec (i.e. 60 Hz)
unsigned char delay_timer;
unsigned char sound_timer; // It makes the computer "beep" as long as it's above 0

uint8_t key;

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
	static int s =0;

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
			
			if(debug_flag)
				printf("Welcome to case 0\n");

			switch(fourth_nibble){
				case 0x0:
					if(debug_flag)
						printf("Welcome to case 00E0\n");
					clear_screen(g);
					break;

				case 0xE:
					if(debug_flag){
						printf("Welcome to case 00E0\n");
						printf("PC's value before is 0x%X\n",(int)registers->PC);
					}

					registers->PC = stack[--s];	

					if(debug_flag)
						printf("PC's value after is 0x%X\n",(int)registers->PC);
					
					break;
			}
			
			break;
		case 0x1:
			/* Valid instructions:
			 * 1NNN => Jumps to address NNN */

			if(debug_flag){
				printf("Welcome to case 1\n");
				printf("PC's value is 0x%X\n",(int)registers->PC);
			}
			registers->PC = NNN;

			break;
		case 0x2:
			/* Valid instructions:
			 * 2NNN => Calls to address NNN. The difference between calling and jumping
			 *         is that in call the address of the current PC will be pushed in 
			 *         the stack and when the function ends (i.e. returned) the value
			 *         will be popped of the stack*/

			if(debug_flag){
				printf("Welcome to case 2NNN\n");
				printf("PC's value is 0x%X\n",(int)registers->PC);
			}

			stack[s++] = registers->PC;
			registers->PC = NNN;

			break;

		case 0x3:
			/* Valid instructions:
			 * 3XNN => Compares Vx and NN (Vx == NN), if true then it 
			 * skips next instruction */

			if(debug_flag){
				printf("Welcome to case 3\n");
				printf("PC's value before is 0x%X\n",(int)registers->PC);
			}
			if(registers->V[X] == NN)
				registers->PC += 2;

			if(debug_flag)
				printf("PC's value after is 0x%X\n",(int)registers->PC);

			break;
		case 0x4:
			/* Valid instructions:
			 * 4XNN => Compares Vx and NN (Vx != NN), if true then 
			 * it skips next instruction */

			if(debug_flag){
				printf("Welcome to case 4\n");
				printf("PC's value before is 0x%X\n",(int)registers->PC);
			}
			if(registers->V[X] != NN)
				registers->PC += 2;

			if(debug_flag)
				printf("PC's value after is 0x%X\n",(int)registers->PC);

			break;
		case 0x5:
			/* Valid instructions:
			 * 5XY0 => Compares Vx and Vy (Vx == Vy), if true then 
			 * it skips next instruction */

			if(debug_flag){
				printf("Welcome to case 5\n");
				printf("PC's value before is 0x%X\n",(int)registers->PC);
			}
			if(registers->V[X] == registers->V[Y])
				registers->PC += 2;

			if(debug_flag)
				printf("PC's value after is 0x%X\n",(int)registers->PC);

			break;
		case 0x6:
			/* Valid instructions:
			 * 6XNN : Sets Vx to NN */

			if(debug_flag){
				printf("Welcome to case 6\n");
				printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
			}

			registers->V[X] = NN;

			if(debug_flag)
				printf("Register %d's value after is 0x%X\n",X,registers->V[X]);

			break;
		case 0x7:
			/* Valid instructions:
			 * 7XNN : Adds Vx to NN */

			if(debug_flag){
				printf("Welcome to case 7\n");
				printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
			}

			registers->V[X] += NN;

			if(debug_flag)
				printf("Register %d's value after is 0x%X\n",X,registers->V[X]);


			break;
		case 0x8:
			/* Valid instructions:
			 * 8XY0 => Set's Vx equal to Vy (Vx = Vy)
			 * 8XY1 => Set's Vx equal to Vx OR Vy (Vx |= Vy)
			 * 8XY2 => Set's Vx equal to Vx AND Vy (Vx &= Vy)
			 * 8XY3 => Set's Vx equal to Vx XOR Vy (Vx ^= Vy)
			 * 8XY4 => Adds Vy  to Vx,if overflow then VF = 1 else VF = 0 (Vx += Vy)
			 * 8XY5 => Subtracts Vx from Vy,if underflow then VF = 0 else VF = 1 (Vx -= Vy)
			 * 8XY6 => Shifts Vx to the right by 1 (Vx >>= 1) , stores LSB before shifting
			 * 	   into Vf
			 * 8XY7 => Subtracts Vy from Vx,if underflow then VF = 0 else VF = 1
			 *	   (Vx= Vy - Vx) 
			 * 8XYE => Shifts Vx to the left by 1 (Vx <<= 1) , sets VF = 1 if MSB of Vx
			 * 	   prior to shifting was present , else VF = 0*/

			
			if(debug_flag)
				printf("Welcome to case 8\n");

			switch(fourth_nibble){
				case 0x0:
					if(debug_flag){
						printf("Welcome to case 8XY0\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}

					registers->V[X] = registers->V[Y];
					
					if(debug_flag)
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);

					break;

				case 0x1:
					if(debug_flag){
						printf("Welcome to case 8XY1\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}

					registers->V[X] |= registers->V[Y];

					if(debug_flag)
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);

					break;
				case 0x2:
					if(debug_flag){
						printf("Welcome to case 8XY2\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}

					registers->V[X] &= registers->V[Y];

					if(debug_flag)
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);

					break;

				case 0x3:
					if(debug_flag){
						printf("Welcome to case 8XY3\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}

					registers->V[X] ^= registers->V[Y];

					if(debug_flag)
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);

					break;

				case 0x4:
					if(debug_flag){
						printf("Welcome to case 8XY4\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}
						
					// An extra variable for storing the sum is used for an extreme
					// case like "80F4",if the VF is set before then the sum would
					// be wrong so hence the result is stored in a different 
					// variable all together.
					int sum = registers->V[X] + registers->V[Y];

					registers->V[X] += registers->V[Y];

					if(sum >= 255)
							registers->V[0xF] = 1;
					else
							registers->V[0xF] = 0;

					if(debug_flag){
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);
						printf("Register F's value after is 0x%X\n",registers->V[0xF]);
					}

					break;

				case 0x5:
					if(debug_flag){
						printf("Welcome to case 8XY5\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
						printf("Register %d's value before is 0x%X\n",Y,registers->V[Y]);
					}
					
					bool _diff = registers->V[X] >= registers->V[Y];
					
					registers->V[X] = registers->V[X] - registers->V[Y];

					if(_diff)
							registers->V[0xF] = 1;
					else
							registers->V[0xF] = 0; // underflow

					if(debug_flag){
						printf("Register %d's value after is 0x%X(%d)\n",X,registers->V[X],registers->V[X]);
						printf("Register %d's value after is 0x%X\n",Y,registers->V[Y]);
						printf("Register F's value after is 0x%X\n",registers->V[0xF]);
					}

					break;

				case 0x6:
					if(debug_flag){
						printf("Welcome to case 8XY6\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}
					
					unsigned short last_bit = 0x1;
					last_bit &= registers->V[X];

					registers->V[X] >>= 1;

					if(last_bit == 1)
						registers->V[0xF] =1;
					else
						registers->V[0xF] =0;

					if(debug_flag){
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);
						printf("Register F's value after is 0x%X\n",registers->V[0xF]);
					}

					break;


				case 0x7:
					if(debug_flag){
						printf("Welcome to case 8XY7\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}
					
					int diff =registers->V[Y] >= registers->V[X]; 

					registers->V[X] = registers->V[Y] - registers->V[X];

					if(diff)
							registers->V[0xF] = 1;
					else
							registers->V[0xF] = 0;

					if(debug_flag){
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);
						printf("Register F's value after is 0x%X\n",registers->V[0xF]);
					}

					break;

				case 0xE:
					if(debug_flag){
						printf("Welcome to case 8XYE\n");
						printf("Register %d's value before is 0x%X\n",X,registers->V[X]);
					}
					
					unsigned short first_bit = 0x80;
					first_bit &= registers->V[X];
					if(debug_flag)
						printf("The num:0b%08b\n",first_bit);
					first_bit >>= 7;

					if(debug_flag)
						printf("The first bit is:%b from the num:0b%08b\n",first_bit,registers->V[X]);

					registers->V[X] <<= 1;

					if(first_bit == 1)
						registers->V[0xF] =1;
					else
						registers->V[0xF] =0;

					if(debug_flag){
						printf("Register %d's value after is 0x%X\n",X,registers->V[X]);
						printf("Register F's value after is 0x%X\n",registers->V[0xF]);
					}

					break;

				default:
					printf("Bad instruction\n");
					break;
			}

			break;

		case 0x9:
			/* Valid instructions:
			 * 9XY0 => Compares Vx and Vy (Vx != NN), if true then it 
			 * skips next instruction */

			if(debug_flag){
				printf("Welcome to case 9\n");
				printf("PC's value before is 0x%X\n",(int)registers->PC);
			}
			if(registers->V[X] != registers->V[Y])
				registers->PC += 2;

			if(debug_flag)
				printf("PC's value after is 0x%X\n",(int)registers->PC);

			break;

		case 0xA:
			/* Valid instructions:
			 * ANNN : Sets I to the address NNN.*/

			if(debug_flag){
				printf("Welcome to case A\n");
				printf("Register value before is 0x%X\n",(int)registers->I);
				printf("mem at the location is:%X\n",memory[registers->I]);
			}

			registers->I = NNN;

			if(debug_flag){
				printf("Register value after is 0x%X\n",(int)registers->I);
				printf("mem at the location is:%X\n",memory[registers->I]);
			}

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

			for(int i = 0; i < N;i++)
				draw(g,registers->V[X],registers->V[Y] + i,memory[(registers->I) + i]);

			break;
		case 0xE:
			/* Valid instructions:
			 * EX9E : Skips the next instruction if the key stored in VX(only consider 
			 *        the lowest nibble) is pressed (usually the next instruction is a jump 
			 *        to skip a code block). (if(key == Vx))
			 * EXA1 : Skips the next instruction if the key stored in VX(only consider 
			 *        the lowest nibble) is not pressed (usually the next instruction is a 
			 *        jump to skip a code block). (if(key != Vx))        */


			if(debug_flag)
				printf("Welcome to case E\n");
			switch(third_nibble){
				case 0x9:
					if(debug_flag){
						printf("Welcome to case EX9E\n");
						printf("PC's value after is 0x%X\n",(int)registers->PC);
					}

					if(key == registers->V[X])
						registers->PC += 2;

					if(debug_flag)
						printf("PC's value after is 0x%X\n",(int)registers->PC);

					break;
					
				case 0xA:
					if(debug_flag){
						printf("Welcome to case EXA1\n");
						printf("PC's value after is 0x%X\n",(int)registers->PC);
					}

					if(key != registers->V[X])
						registers->PC += 2;

					if(debug_flag)
						printf("PC's value after is 0x%X\n",(int)registers->PC);

					break;
			}

			break;
		case 0xF:
			/* Valid instructions:
			 * FX1E: Adds Vx to I.( I += Vx) 
			 * FX33: Stores BCD of Vx into I. 100's at I,10's at I+1,1's at I +2.
			 * FX55: Stores vals from V0 to Vx in memory starting at address I. Offset of 
			 *       I is increased by 1 after a val is written into it but I itself isn't
			 *       changed.
			 * FX65: Fills vals from V0 to Vx from memory starting at address I. Offset of 
			 *       I is increased by 1 after a val is written into it but I itself isn't
			 *       changed.
			 */

			if(debug_flag){
				printf("Welcome to case F\n");
				printf("Register I's value before is 0x%X\n",(int)registers->I);
				printf("mem at the location is:%X\n",memory[registers->I]);
			}

			switch(third_nibble){
				case 0x1:
					switch(fourth_nibble){
						case 0xE:
							if(debug_flag)
								printf("Welcome to case FX1E\n");

							registers->I += registers->V[X];

							break;
					}
					
					break;
				case 0x3:
					if(debug_flag){
						printf("Welcome to case FX33\n");
						_memoryframe(registers->I,registers->I + 2);
						printf("val at V%d is 0x%X\n",X,registers->V[X]);
					}
					
					for(int i = 0;i < 3;i++)
						memory[registers->I + i] = 0;

					int num = registers->V[X];
					int _i = 2;

					while(num != 0){
						if(debug_flag)
							printf("Num now is:%d with digit: %d being stored at 0x%X\n",num,num%10,registers-> I + _i);
						memory[registers->I + _i--] = num%10;
        					num = num / 10;
    					}

					if(debug_flag)
						_memoryframe(registers->I,registers->I + 2);
					break;

				case 0x5:
					if(debug_flag){
						printf("Welcome to case FX55\n");
						_memoryframe(registers->I,registers->I + X);
						for(int i = 0;i <= X;i++)
							printf("val at V%d is %X\n",i,registers->V[i]);
					}

					for(int i = 0;i <= X;i++)
						memory[(registers->I) + i] = registers->V[i];
					
					if(debug_flag)
						_memoryframe(registers->I,registers->I + X);
					break;

				case 0x6:
					if(debug_flag){
						printf("Welcome to case FX65\n");
						_memoryframe(registers->I,registers->I + X);
						for(int i = 0;i <= X;i++)
							printf("val at V%d is %X\n",i,registers->V[i]);
					}

					for(int i = 0;i <= X;i++)
						registers->V[i] = memory[(registers->I) + i]; 
					
					if(debug_flag)
						_memoryframe(registers->I,registers->I + X);
					break;
			}

			if(debug_flag){
				printf("Register I's value after is 0x%X\n",(int)registers->I);
				printf("mem at the location is:%X\n",memory[registers->I]);
			}

			break;

		default:
			printf("First nibble after: %016b\n",first_nibble);
			break;
	}

	logmsg("execute",false,debug_flag);
}

void game_run(struct Game *g , _registers *registers){
	while(g->is_running){
		key = 16; //invalid
		key = game_events(g);
		printf("=> In game run ,  %X is pressed\n",key);
		//game_draw(g);
		execute(fetch(registers),registers);
		if(registers-> PC == 0x333)
			g->is_running = false;
		SDL_Delay(1/500); // i.e. 60Hz
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

int main(int argc,char** agrv){
	_registers registers;
	registers.PC = 0x200; // starting from the unreserved section
	_fontset();
	_memoryframe(0x050,0x200);	
	//printf("%d\n",EXIT_FAILURE);
	bool exit_status = EXIT_FAILURE;

	//printf("game: %p\n",g);
	
	if(argc == 1)
		_fillopcode();
	else
		if(!(load_ROM("ROMs/4_flags.ch8")))
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

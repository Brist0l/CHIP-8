#include<string.h>
#include<stdio.h>
#include<stdbool.h>

bool debug_flag = true;

// CHIP-8 has 16 8-bit registers (V0 - VF)  
typedef struct _registers{ 
	char V0;
	char V1;
	char V2;
	char V3;
	char V4;
	char V5;
	char V6;
	char V7;
	char V8;
	char V9;
	char VA;
	char VB;
	char VC;
	char VD;
	char VE;
	char VF; // doubles as a flag register
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

void logmsg(const char* function_name,bool start,bool debug_flag);
void _fontset();
void _memoryframe(unsigned _BitInt(12) start,unsigned _BitInt(12) end);
void _fillopcode();
const unsigned short fetch(_registers registers);

void logmsg(const char* function_name,bool start,bool debug_flag){
	if(!debug_flag)
		return;

	if(start)
		printf("\n=====function %s START=====\n",function_name);
	else
		printf("=====function %s END=====\n",function_name);
}

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
	
	logmsg("_fontset",true,debug_flag);

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

// View the memory locations from a starting address to an ending address
void _memoryframe(unsigned _BitInt(12) start,unsigned _BitInt(12) end){
	logmsg("_memoryframe",true,debug_flag);
	if(debug_flag)
		for(unsigned _BitInt(12) i = start; i < end;i++)
			if(memory[i] != 0)
				printf("val is: %02x at 0x%04x\n",(int) memory[i],(int) i);
	logmsg("_memoryframe",false,debug_flag);
}

void _fillopcode(){
	// Filling it with a const opcode , 00EE => basically it's C's `return`
	// So all the opcodes are 2 bytes long and it's stored in big-endian format 
	memory[0x200] = 0x00;
	memory[0x201] = 0xE1;
}

const unsigned short fetch(_registers registers){
	logmsg("fetch",true,debug_flag);
	_fillopcode();

	unsigned short opcode = 0x0;
	unsigned short MSB = memory[registers.PC];
	MSB <<= 8; // shifting the number into MSB side 
	unsigned short LSB = memory[registers.PC + 1];
	opcode = opcode | MSB | LSB; 

	registers.PC += 2; // Increment PC by 2 cuz 2 consequitive bytes of memory has been accessed	
			   
	if(debug_flag){
		printf("The opcode is:%04X(dec equivalent:%06d and bin is %016b)\n",opcode,opcode,opcode);
		printf("The PC is now:%x\n",(int)registers.PC);
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
void execute(const unsigned short opcode){
	logmsg("execute",true,debug_flag);
	unsigned short first_nibble = 0xF000;
	unsigned short second_nibble = 0x0F00;
	unsigned short third_nibble = 0x00F0;
	unsigned short fourth_nibble = 0x000F;

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

	if(debug_flag){
		printf("First nibble after: %016b(%X)\n",first_nibble,first_nibble);
		printf("Second nibble after: %016b(%X)\n",second_nibble,second_nibble);
		printf("Third nibble after: %016b(%X)\n",third_nibble,third_nibble);
		printf("Fourth nibble after: %016b(%X)\n",fourth_nibble,fourth_nibble);
	}

	switch(first_nibble){
		case 0:
			/* Valid instructions:
			 * 00E0 => Clears the screen
			 * 00EE => Returns from the subroutine
			 */

			printf("Welcome to case 0 bitch\n");
			switch(fourth_nibble){
				case 0:
					printf("Clearing screen\n");
					break;
				case 0xE:
					printf("returing\n");
					break;
				default:
					printf("Bad instruction\n");
			}
			
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 1:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 2:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 3:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 4:
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 5:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 6:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 7:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 8:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 9:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xA:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xB:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xC:
			printf("Welcome to case C bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xD:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xE:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;
		case 0xF:
			printf("Welcome to case 0 bitch\n");
			printf("First nibble after: %016b\n",first_nibble);
			break;

		default:
			printf("First nibble after: %016b\n",first_nibble);
			break;
	}

	logmsg("execute",false,debug_flag);
}

int main(){
	_registers registers;
	registers.PC = 0x200; // starting from the unreserved section
	_fontset();
	_memoryframe(0x050,0x200);	
	execute(fetch(registers));
}

#include<string.h>
#include<stdio.h>

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

unsigned char stack[48]; // 48 bytes worth of stack
// 4096 bytes worth of stack . first 512(0x200) bytes are reserved i.e. the opcodes need to be loaded
// from 0x200.
unsigned char memory[0x1000]; 

// Both the timers need to be decremented by 1 , 60 times per sec (i.e. 60 Hz)
unsigned char delay_timer;
unsigned char sound_timer; // It makes the computer "beep" as long as it's above 0

void font_set();
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

void font_set(){
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
	};

	int pos = 0x050;
	for(int i = 0; i < 16; i++){
		for(int j = 0;j < 5;j++){
//			printf("val is: %x at %d\n",fonts[i][j],pos);
			memset(&memory[pos++],fonts[i][j],sizeof(fonts[i][j]));
			//pos += 1;
		}
	}
}

void fetch(_registers registers){
	registers.PC += 2;	
}

int main(){
	_registers registers;
	registers.PC = 0x200; // starting from the unreserved section
	font_set();
	for(int i = 0; i < 0x1000;i++)
		if(memory[i] != 0)
			printf("%d => %x\n",i,memory[i]);
}

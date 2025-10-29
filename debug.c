#include<stdbool.h>
#include<stdio.h>

extern bool debug_flag;
extern unsigned char memory[0x1000];

void logmsg(const char* function_name,bool start,bool debug_flag){
	if(!debug_flag)
		return;

	if(start)
		printf("\n=====function %s START=====\n",function_name);
	else
		printf("=====function %s END=====\n",function_name);
}

// View the memory locations from a starting address to an ending address
void _memoryframe(unsigned _BitInt(12) start,unsigned _BitInt(12) end){
	logmsg("_memoryframe",true,debug_flag);

	if(debug_flag)
		for(unsigned _BitInt(12) i = start; i <= end;i++)
				printf("val is: 0x%02x at 0x%04x\n",(int) memory[i],(int) i);

	logmsg("_memoryframe",false,debug_flag);
}

void _fillopcode(){
	// Filling it with a const opcode , 00EE => basically it's C's `return`
	// So all the opcodes are 2 bytes long and it's stored in big-endian format 
	memory[0x200] = 0x60;
	memory[0x201] = 0x05;
	memory[0x202] = 0x61;
	memory[0x203] = 0x05;
	memory[0x204] = 0xF1;
	memory[0x205] = 0x29;
	memory[0x206] = 0xD0;
	memory[0x207] = 0x15;
}

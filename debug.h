#ifndef DEBUG_H
#define DEBUG_H

void logmsg(const char* function_name,bool start,bool debug_flag);
void _memoryframe(unsigned _BitInt(12) start,unsigned _BitInt(12) end);
void _fillopcode();

#endif

#ifndef MEMORY_CHECKS_H
#define MEMORY_CHECKS_H

#include "memory.h"

void printMemory(unsigned int s,unsigned int e, Memory m);
int is_mem_address_not_aligned(unsigned int memoryAddress, int alignment);
char* readWordAsString(unsigned long memoryAddress, Memory m);
void printMemory_to_file(unsigned int start,unsigned int end, Memory m, ofstream& outfile);

#endif

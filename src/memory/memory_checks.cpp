#include "processor/constants.h"
#include "memory_checks.h"
#include <iostream>
#include <fstream>
using namespace std;
//#include "memory/memory.cpp"
/*
 * Returns 1, if memoryAddress is not aligned on <alignment> boundary.
 * Returns 0, if memoryAddress is aligned on <alignment> boundary.
 * It's useful to prevent loading mis-aligned memoryAddress
 * in double word (DWORD) instructions by raising
 * memory_address_not_aligned traps.
 */
int is_mem_address_not_aligned(unsigned int memoryAddress, int alignment)
{
    switch(alignment)
    {
        case 2: if((((unsigned int)(memoryAddress / 2)) * 2) == memoryAddress) return 0; else return 1;
        case 4: if((((unsigned int)(memoryAddress / 4)) * 4) == memoryAddress) return 0; else return 1;
        case 8: if((((unsigned int)(memoryAddress / 8)) * 8) == memoryAddress) return 0; else return 1;
    }
    return 0;
}
void printMemory(unsigned int start,unsigned int end, Memory m){
    unsigned int bound = 0;
    if (end < MAX_MEM)
    {
        bound = end;
    }
    else
    {
            bound = MAX_MEM;
    }
    
    for(unsigned int i=start; i<bound; i=i+4){
        xout << m.getWord (i) << endl;
    }
}

void printMemory_to_file(unsigned int start,unsigned int end, Memory m, ofstream& outfile){
    unsigned int bound = 0;
    if (end < MAX_MEM)
    {
        bound = end;
    }
    else
    {
            bound = MAX_MEM;
    }
    outfile<<"Clock:"<<clock_cycles<<"\n";
    for(unsigned int i=start; i<bound; i=i+4){
        // fprintf(outfile,"\n%x\n", i );
        char hi[32];
        sprintf(hi,"%X", i);
        outfile<<"["<<hi<<"]:"<<m.getWord(i)<<endl;
    }
    outfile<<"\n";
}

char* readWordAsString(unsigned long memoryAddress, Memory memory)
{
	char* cpuInstruction = new char [4];
	cpuInstruction[0] = memory.getByte(memoryAddress++);          // Read the first byte.
	cpuInstruction[1] = memory.getByte(memoryAddress++);          // Read the second byte.
	cpuInstruction[2] = memory.getByte(memoryAddress++);          // Read the third byte.
	cpuInstruction[3] = memory.getByte(memoryAddress);            // Read the fourth byte.
	return cpuInstruction;
}
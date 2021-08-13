#ifndef SIM_GLOBALS_H
#define SIM_GLOBALS_H

#include <iostream>

extern long long unsigned int clock_cycles;
extern  int SIM_LOOP_SIGNAL;
extern int MEM_LAT;
extern int MEMDUMP_MAX;
extern int Quiet;
extern std::ofstream xout;

///STATS
extern long long unsigned int nINS; // #InsExecs
extern long long unsigned int nMR; // #memoryReads
extern long long unsigned int nMW; // #memoryWrites
extern long long unsigned int nBT; // #Branch Taken {BranchIntegerInstructions,BranchFloatInstructions,JumpAndLink,RETT}


#endif

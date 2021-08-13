#ifndef XC_RW_LATCH_H
#define XC_RW_LATCH_H

#include "processor/instruction.h"
#include "processor/pipeline/pheader.h"

class XC_RW_latch 
{
    public:

    bool isRW_Enabled;
    bool isregWrite;
    instruction Ins;
    int retVal; /// not needed
    //// other reg operands required later for pass to RW
    // regType RD;
    // regType RD_val;
    
    RW_pack rw_pack;
    
    XC_RW_latch();

};

#endif

#ifndef ME_XC_LATCH_H
#define ME_XC_LATCH_H

#include "processor/instruction.h"
#include "processor/pipeline/pheader.h"
// #include "generic/utility.h"

class ME_XC_latch 
{
    public:
        bool isXC_Enabled;
        instruction Ins;
        int retVal;
        
        //// other reg operands required later for pass to RW
        
        RW_pack rw_pack;
        XC_pack xc_pack;

        ME_XC_latch();
};

#endif

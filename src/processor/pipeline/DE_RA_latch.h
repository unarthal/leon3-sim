#ifndef DE_RA_LATCH_H
#define DE_RA_LATCH_H

#include "processor/instruction.h"

class DE_RA_latch 
{
    public:
        bool isRA_Enabled;
        instruction decodedIns;
        
        DE_RA_latch();
};

#endif

#ifndef EX_ME_LATCH_H
#define EX_ME_LATCH_H

#include "processor/instruction.h"
#include "processor/pipeline/pheader.h"
// #include "generic/utility.h"

//// Mem Instructions address latches
// LoadIntegerInstructions
// LoadFloatingPointInstructions
// StoreIntegerInstructions
// StoreFloatInstructions
// AtomicLoadStoreUnsignedByte : both load and store are done from/to memory
// SWAP: write on both memory and register

//// (Write) Destination Regs latch

//// Exceptions (to be done last)

class EX_ME_latch
{
    public:
        bool isME_Enabled;
        bool isMemIns;

        instruction Ins;
        int retVal;
        
        addrType m_addr;
        bool isInt;
        bool isFloat;
        bool isAtomic;
        bool isLoad;
        bool isStore;
        bool isSwap;

        /// to Store
        int regRD;
        int regNextRD;

        int FSR;

        RW_pack rw_pack;
        XC_pack xc_pack;

        int nReq = 0;
        int countReq=0;

        char byte;
        char* halfword0;
        char* halfword4;
        unsigned long word0;

        EX_ME_latch();
};

#endif

#ifndef PHEADER_H
#define PHEADER_H

#include "processor/constants.h"

// FIXME: moved XC_pack and RW_pack here 
// from utility.h. Check if this is fine
struct XC_pack
{
    bool isError;
    int psr;
    int tbr;
    int nWP;
    // unsigned int S;
    // unsigned int cwp;
    // unsigned int et;
    addrType PC;
    addrType nPC;
};

struct RW_pack
{
    bool isIntReg;
    bool isFltReg;
    bool isASR;
    bool isPSR;
    bool isFSR;
    bool isTBR;  
    bool isY;
    bool isWIM;
    bool isCounter;
    bool isTrapSaveCounter;
/////////////////////////////
    regType RD;
    regType RD_val;
    int F_nWords;

    regType F_RD1;
    regType F_RD2;
    regType F_RD3;
    regType F_RD4;
    regType F_RD1_val;
    regType F_RD2_val;
    regType F_RD3_val;
    regType F_RD4_val;

    addrType PC;
    addrType nPC;

    int PSR;
    int FSR;
    int TBR;

    int Y;
    int WIM;
};




// #include "FE-DE_latch.h"
// #include "DE-RA_latch.h"
// #include"RA-EX_latch.h"
// #include "ME-XC_latch.h"
// #include "XC-RW_latch.h"
// #include "XC-RW_latch.h"


// #include "Element.h"

#endif

#ifndef RA_EX_LATCH_H
#define RA_EX_LATCH_H

#include "processor/instruction.h"
#include "processor/pipeline/pheader.h"
// #include "generic/utility.h"

class RA_EX_latch 
{
    public:
        bool isEX_Enabled;
        instruction l_Ins;

        ////Integer
        int l_regRS1;
        int l_reg_or_imm;
        int l_regRS2;
        int l_regRD; //common
        int l_regNextRD; //common

        ////Floating Point
        int l_regFs1_1;
        int l_regFs1_2;
        int l_regFs1_3;
        int l_regFs1_4;
        int l_regFs2_1;
        int l_regFs2_2;
        int l_regFs2_3;
        int l_regFs2_4;

        int l_PSR_EF;
        int l_PSR_ET;
        int l_PSR_S;
        int l_PSR_PS;

        int l_WIM;
        int l_PSR;
        int l_FSR;
        int l_ASR;
        int l_TBR;
        int l_Y;

        int l_PSR_icc_N;
        int l_PSR_icc_Z;
        int l_PSR_icc_V;
        int l_PSR_icc_C;
        int l_FSR_RD;

        int l_CWP;
        int l_nWP;

        int l_FSR_fcc;
        int l_PC;
        int l_nPC;
        //  int l_ASRreg;
        
        XC_pack xc_pack;
        RA_EX_latch();

};

#endif

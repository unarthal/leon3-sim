#include "ME_XC_latch.h"
#include "generic/utility.h"
#include "XC_RW_latch.h"
#include "XC.h"
#include <strings.h>
#include <iostream>

extern  int SIM_LOOP_SIGNAL;

XC::XC( ME_XC_latch* mexc, XC_RW_latch* xcrw) 
{
    mexc_latch = mexc;
    xcrw_latch = xcrw;    
}

void XC::handleEvent()
{
}

int XC::trap(int memAddr)
{
    int t_psr=mexc_latch->xc_pack.psr;
    int t_tbr=mexc_latch->xc_pack.tbr;
    
    unsigned int ET = getBits(mexc_latch->xc_pack.psr,0, PSR_ET_l, PSR_ET_r);
    unsigned int S = getBits(mexc_latch->xc_pack.psr,0, PSR_S_l, PSR_S_r);
    unsigned int CWP = getBits(mexc_latch->xc_pack.psr, 0, PSR_CWP_l, PSR_CWP_r);

    if(ET==0) 
    {
        return error_mode;
    }

    t_psr = modifyBits(0, t_psr, PSR_ET_l, PSR_ET_r);
    t_psr = modifyBits(S, t_psr, PSR_PS_l, PSR_PS_r);
    t_psr = modifyBits(1, t_psr, PSR_S_l, PSR_S_r);

    xout << "intrap getocwp:" <<CWP<< endl;

    t_psr = modifyBits( ((CWP-1)%mexc_latch->xc_pack.nWP), t_psr, PSR_CWP_l, PSR_CWP_r);
    if (CWP < 0)
    {
        t_psr = modifyBits( (CWP + mexc_latch->xc_pack.nWP), t_psr, PSR_CWP_l, PSR_CWP_r);
    }

    xout << "intrap getncwp:" <<CWP << endl;     
    
    xcrw_latch->rw_pack.isTrapSaveCounter=true;
    t_tbr = modifyBits(memAddr, t_tbr, TBR_TT_l, TBR_TT_r);
    xcrw_latch->rw_pack.TBR = memAddr;       
    xcrw_latch->rw_pack.PC = mexc_latch->xc_pack.tbr;
    xcrw_latch->rw_pack.nPC = mexc_latch->xc_pack.tbr + 4;
    xcrw_latch->rw_pack.isCounter = true;

    xout << "TRAP PC:" << mexc_latch->xc_pack.PC << endl;
    xcrw_latch->rw_pack.PSR = t_psr;
    xcrw_latch->rw_pack.isPSR = true;
    xcrw_latch->rw_pack.isTBR = true;
    return RET_SUCCESS;
}

void XC::perform() 
{
    if (mexc_latch->isXC_Enabled == false)
    {
        xout<<"XC STALL\n"; 
        return;
    }
    else
    {
        xout<<"XC start\n";
    }

    instruction* ins = &mexc_latch->Ins;
    
    int ret = mexc_latch->retVal;
    if(ret != 1)
    {
        xout << "Trap generated" << endl;
        int is_error=trap(ret); // (6.XC-eve)
        // return is_error; /// signal to Sim loop to break if -1 i.e ET = 0 
        SIM_LOOP_SIGNAL = is_error;
    } 
    else 
    {
        // No Traps 
        xcrw_latch->rw_pack = mexc_latch->rw_pack;
        xcrw_latch->isregWrite = true;
        SIM_LOOP_SIGNAL = RET_SUCCESS;
    }

    bzero(mexc_latch, sizeof(ME_XC_latch));
    mexc_latch->isXC_Enabled = false;
    xcrw_latch->isRW_Enabled =true;
}

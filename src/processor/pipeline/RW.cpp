#include <strings.h>
#include <iostream>
#include "RW.h"
#include "generic/utility.h"

RW::RW(Register* reg, XC_RW_latch* xcrw, RW_FE_latch* rwfe) 
{
    sregister =  reg;
    xcrw_latch = xcrw;
    rwfe_latch = rwfe;           
}

void RW::handleEvent()
{
}

void RW::perform() 
{
    if (xcrw_latch->isRW_Enabled == false)
    {
        xout<<"RW STALL\n";
        return;
    }
    else
    {
        xout<<"RW start\n";
    }

    if(xcrw_latch->isregWrite == true)
    {
        if (xcrw_latch->rw_pack.isPSR == true)
        {
            sregister->SRegisters.psr->setPSR(xcrw_latch->rw_pack.PSR);
        }
        
        if(xcrw_latch->rw_pack.isIntReg)
        {
            regType RD = xcrw_latch->rw_pack.RD;
            regType RD_val = xcrw_latch->rw_pack.RD_val;
            sregister->setRegister(RD, RD_val);  
        }
        
        if(xcrw_latch->rw_pack.isFltReg == true)
        {
            int nWords = xcrw_latch->rw_pack.F_nWords;
            switch (nWords)
            {
                case 1: //word
                    sregister->setFloatingRegister (xcrw_latch->rw_pack.F_RD1, 
                            xcrw_latch->rw_pack.F_RD1_val);
                    break;

                case 2: //double word
                    sregister->setFloatingRegister(xcrw_latch->rw_pack.F_RD1, 
                            xcrw_latch->rw_pack.F_RD1_val);
                    sregister->setFloatingRegister(xcrw_latch->rw_pack.F_RD2, 
                            xcrw_latch->rw_pack.F_RD2_val);
                    break;

                case 4: //quad word
                    sregister->setFloatingRegister(xcrw_latch->rw_pack.F_RD1, 
                            xcrw_latch->rw_pack.F_RD1_val);
                    sregister->setFloatingRegister(xcrw_latch->rw_pack.F_RD2, 
                            xcrw_latch->rw_pack.F_RD2_val);
                    sregister->setFloatingRegister(xcrw_latch->rw_pack.F_RD3, 
                            xcrw_latch->rw_pack.F_RD3_val);
                    sregister->setFloatingRegister(xcrw_latch->rw_pack.F_RD4, 
                            xcrw_latch->rw_pack.F_RD4_val);
                    break;               

                default:
                    xout << "Unknown no. of Words to write in FPU regs:" << nWords << "\n";
                    break;
            }
        }

        if(xcrw_latch->rw_pack.isCounter == true)
        { 
            //To check is PC and nPC are set together every time
            sregister->setPC(xcrw_latch->rw_pack.PC);
            sregister->setnPC(xcrw_latch->rw_pack.nPC);
        }
        
        if (xcrw_latch->rw_pack.isPSR == true)
        {
            sregister->SRegisters.psr->setPSR(xcrw_latch->rw_pack.PSR);
        }

        if (xcrw_latch->rw_pack.isFSR == true) 
        {
            sregister->SRegisters.fsr->setFSR(xcrw_latch->rw_pack.FSR);
        }

        if (xcrw_latch->rw_pack.isTBR == true)
        {
            sregister->SRegisters.tbr->setTbr(xcrw_latch->rw_pack.TBR);
        }

        if (xcrw_latch->rw_pack.isY == true)
        {
            sregister->setY(xcrw_latch->rw_pack.Y);
        }

        if (xcrw_latch->rw_pack.isWIM == true)
        {
            sregister->SRegisters.wim = xcrw_latch->rw_pack.WIM;
        }
        
        if (xcrw_latch->rw_pack.isTrapSaveCounter == true)
        {
            sregister->setRegister(17, xcrw_latch->rw_pack.PC);
            sregister->setRegister(18, xcrw_latch->rw_pack.nPC);
        }
    }

    bzero(xcrw_latch, sizeof(XC_RW_latch));
    xcrw_latch->isRW_Enabled =false;
    rwfe_latch->isFE_Enabled = true;
}


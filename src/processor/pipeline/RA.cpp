#include <strings.h>
#include "generic/utility.h"
#include"RA_EX_latch.h"
#include"RA.h"

RA::RA(Register* reg, RA_EX_latch* raex, DE_RA_latch* dera)
{
    sregister =  reg;
    raex_latch = raex;
    dera_latch = dera;
}

void RA::handleEvent()
{
}

void RA::perform ()
{
    if(dera_latch->isRA_Enabled == false){xout<<"RA STALL\n";return;}
    else{xout<<"RA start\n";}

    raex_latch->l_Ins = dera_latch->decodedIns;

    raex_latch->l_PSR = sregister->SRegisters.psr->getPSR();
    raex_latch->l_FSR = sregister->SRegisters.fsr->getFSR();
    raex_latch->l_ASR = sregister->getASRRegister(dera_latch->decodedIns.rs1); //hardcoded to rs1 as in ReadStateRegisterInstructions
    raex_latch->l_TBR = sregister->SRegisters.tbr->getTbr();


    raex_latch->l_PSR_EF = sregister->SRegisters.psr->getEf();
    raex_latch->l_PSR_ET = sregister->SRegisters.psr->getEt();
    raex_latch->l_PSR_S = sregister->SRegisters.psr->getS();
    raex_latch->l_PSR_PS = sregister->SRegisters.psr->getS();

    raex_latch->l_FSR_fcc = sregister->SRegisters.fsr->getFcc();
    raex_latch->l_FSR_RD = sregister->SRegisters.fsr->getRd();

    raex_latch->l_WIM = sregister->SRegisters.wim;
    raex_latch->l_CWP = sregister->SRegisters.psr->getCwp();
    raex_latch->l_nWP = sregister->SRegisters.totalregisterWindows;
    raex_latch->l_PC = sregister->getPC();
    raex_latch->l_nPC = sregister->getnPC();


    ///processor state register icc codes
    raex_latch->l_PSR_icc_N = sregister->SRegisters.psr->getN();
    raex_latch->l_PSR_icc_Z = sregister->SRegisters.psr->getZ();
    raex_latch->l_PSR_icc_V = sregister->SRegisters.psr->getV();
    raex_latch->l_PSR_icc_C = sregister->SRegisters.psr->getC();

    ///Multiply/Divide Register (MSB 32 bits of product/dividend)
    raex_latch->l_Y = sregister->getY();


    if (dera_latch->decodedIns.op == 1 ){ //f1
        //No int/flt regload required
        //only pc ang reg15 stp are changed
    } else if (dera_latch->decodedIns.op == 0){ //f2 SETHI/Branches
        if (dera_latch->decodedIns.op2 == 4){
            // SETHI (reg write to rd only)
            //No int/flt regload required
        } else if (dera_latch->decodedIns.op2 == 2 ){
            //Integer Branch
            //No int/flt regload required
            // only pc,npc, psr 
        } else if (dera_latch->decodedIns.op2 == 6 ){
            //Floatingpoint Branch
            //No int/flt regload required
            // only pc,npc, psr
        }

    } else if (dera_latch->decodedIns.op == 2){ //f3 Arith/Logic
        if (dera_latch->decodedIns.op3 == 52 || dera_latch->decodedIns.op3 == 53 ){ // FP Instructions
            raex_latch->l_regFs1_1 = sregister->getFloatingRegister(dera_latch->decodedIns.rs1);
            raex_latch->l_regFs1_2 = sregister->getFloatingRegister(dera_latch->decodedIns.rs1+1);
            raex_latch->l_regFs1_3 = sregister->getFloatingRegister(dera_latch->decodedIns.rs1+2);
            raex_latch->l_regFs1_4 = sregister->getFloatingRegister(dera_latch->decodedIns.rs1+3);

            raex_latch->l_regFs2_1 = sregister->getFloatingRegister(dera_latch->decodedIns.rs2);
            raex_latch->l_regFs2_2 = sregister->getFloatingRegister(dera_latch->decodedIns.rs2+1);
            raex_latch->l_regFs2_3 = sregister->getFloatingRegister(dera_latch->decodedIns.rs2+2);
            raex_latch->l_regFs2_4 = sregister->getFloatingRegister(dera_latch->decodedIns.rs2+3);

            raex_latch->l_regRD = sregister->getFloatingRegister(dera_latch->decodedIns.rd);
            raex_latch->l_regNextRD = sregister->getFloatingRegister(dera_latch->decodedIns.rd+1); 

        } else { // Integer Instructions (to CHECK )
            if(dera_latch->decodedIns.op3 == 58){ //Trap on ICC (imm7 case)
                raex_latch->l_regRS1 = sregister->getRegister(dera_latch->decodedIns.rs1);
                raex_latch->l_reg_or_imm = dera_latch->decodedIns.i ? dera_latch->decodedIns.imm7 : sregister->getRegister(dera_latch->decodedIns.rs2);
                // icc's NZVC common above
            } else {
                raex_latch->l_regRS1 = sregister->getRegister(dera_latch->decodedIns.rs1);
                raex_latch->l_reg_or_imm = dera_latch->decodedIns.i ? dera_latch->decodedIns.simm13 : sregister->getRegister(dera_latch->decodedIns.rs2);
                if(dera_latch->decodedIns.i==0){
                raex_latch->l_regRS2 = sregister->getRegister(dera_latch->decodedIns.rs2);
                }
                raex_latch->l_regRD = sregister->getRegister(dera_latch->decodedIns.rd);
                raex_latch->l_regNextRD = sregister->getRegister(dera_latch->decodedIns.rd+1); // to check
            }



        }
    // Co-Processor Instructions Not Implemented 

    } else if (dera_latch->decodedIns.op == 3){ //f3 Memory
        raex_latch->l_regRS1 = sregister->getRegister(dera_latch->decodedIns.rs1);
        raex_latch->l_reg_or_imm = dera_latch->decodedIns.i ? dera_latch->decodedIns.simm13 : sregister->getRegister(dera_latch->decodedIns.rs2);
        if(dera_latch->decodedIns.i==0){
            raex_latch->l_regRS2 = sregister->getRegister(dera_latch->decodedIns.rs2);
        }

        if(dera_latch->decodedIns.op3==36 || dera_latch->decodedIns.op3==39 || dera_latch->decodedIns.op3==37 || dera_latch->decodedIns.op3==38){ //// Store float point
            raex_latch->l_regRD = sregister->getFloatingRegister(dera_latch->decodedIns.rd);
            raex_latch->l_regNextRD = sregister->getFloatingRegister(dera_latch->decodedIns.rd+1);
        } else { //// store integer or load int/flt
            raex_latch->l_regRD = sregister->getRegister(dera_latch->decodedIns.rd);
            raex_latch->l_regNextRD = sregister->getRegister(dera_latch->decodedIns.rd+1);
        }

    }


    ////FOR XC line
    raex_latch->xc_pack.psr = sregister->SRegisters.psr->getPSR();
    raex_latch->xc_pack.tbr = sregister->SRegisters.tbr->getTbr();



    raex_latch->xc_pack.PC = sregister->getPC();
    raex_latch->xc_pack.nPC = sregister->getnPC();

    raex_latch->xc_pack.tbr = sregister->SRegisters.tbr->getTbr();

    raex_latch->xc_pack.nWP = sregister->SRegisters.totalregisterWindows;

    bzero(dera_latch, sizeof(DE_RA_latch));
    dera_latch->isRA_Enabled = false;
    raex_latch->isEX_Enabled = true;


}


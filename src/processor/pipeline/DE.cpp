#include <strings.h>
#include "generic/utility.h"
#include "DE.h"
#include "DE_RA_latch.h"

DE::DE (FE_DE_latch* fede, DE_RA_latch* dera)
{
    fede_latch = fede;
    dera_latch = dera;        
}

void DE::handleEvent()
{
}

void DE::perform ()
{
    if (fede_latch->isDE_Enabled == false)
    {
        xout<<"DE STALL\n"; 
        return;
    }
    else
    {
        xout<<"DE start\n";
    }
    
    unsigned int instructionWord;

    int sign_extended_disp22, sign_extended_simm13;

    xout <<"\nDECODING INSTRUCTION" <<endl;

    // big endian to litte endian
    instructionWord=convertBtoL(fede_latch->Ins); 

    bzero(&dera_latch->decodedIns, sizeof(instruction)); 
    
    dera_latch->decodedIns.op = 
        extract(instructionWord,30,31);

    if (dera_latch->decodedIns.op == 1) // Format - I instruction
    {
        dera_latch->decodedIns.disp30 = 
            extract(instructionWord,0,29);
    }
    else if (dera_latch->decodedIns.op == 0) // Format - II instruction
    {
        dera_latch->decodedIns.op2 = 
            extract(instructionWord,22,24);

        //SETHI and NOP
        //NOP <- SETHI with rd==0 and imm22==0
        if(dera_latch->decodedIns.op2 == 4)
        {
            dera_latch->decodedIns.rd = 
                extract(instructionWord,25,29);

            dera_latch->decodedIns.imm22 = 
                extract(instructionWord,0,21);

        }
        else
        {
            dera_latch->decodedIns.a = 
                extract(instructionWord,29,29);
            dera_latch->decodedIns.cond = 
                extract(instructionWord,25,28);

            unsigned int disp22 = 
                extract(instructionWord,0,21);

            sign_extended_disp22 = 
                (disp22 & 0x3FFFFF) | 
                ((disp22 & 0x20000) ? 
                    0xFFC00000 : 0); 

            dera_latch->decodedIns.disp22 = 
                sign_extended_disp22;
        }
    }
    else if (dera_latch->decodedIns.op == 2 || 
             dera_latch->decodedIns.op == 3) //Format - III instruction
    {
        dera_latch->decodedIns.rd = 
            extract(instructionWord,25,29);

        dera_latch->decodedIns.op3 = 
            extract(instructionWord,19,24);

        dera_latch->decodedIns.rs1 = 
            extract(instructionWord,14,18);
        
        // TODO: check if the below two 
        // assignments are correct
        dera_latch->decodedIns.opf = 
            extract(instructionWord,5,13); 

        dera_latch->decodedIns.rs2 = 
            extract(instructionWord,0,4);
        // end TODO
        
        dera_latch->decodedIns.i = 
            extract(instructionWord,13,13);
        
        if (dera_latch->decodedIns.i == 1)
        {
            unsigned int simm13 = 
                extract(instructionWord,0,12);

            sign_extended_simm13 = 
                (simm13 & 0x1FFF) | 
                ((simm13 & 0x1000) ? 
                 0xFFFFE000 : 0);
            
            //TODO: Check if sign-extn is reqd.
            dera_latch->decodedIns.simm13 = 
                sign_extended_simm13;

            dera_latch->decodedIns.imm7 = 
                extract(instructionWord,0,6);
        }
        else
        {
            dera_latch->decodedIns.asi = 
                extract(instructionWord,5,12);

            dera_latch->decodedIns.rs2 = 
                extract(instructionWord,0,4);
        }
    }
    
    xout << "\nDECODING INSTRUCTION COMPLETE\n";
    
    bzero(fede_latch,sizeof(FE_DE_latch));
    fede_latch->isDE_Enabled = false;
    dera_latch->isRA_Enabled = true;
}

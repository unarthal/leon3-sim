#include <math.h>
#include <cfenv> // for fesetround() and FE_* macros
#include <iostream> // for xout and endl
#include <iomanip> // for setprecision()
#include <strings.h>
#include "ME.h"
#include "ME_XC_latch.h"
#include "EX_ME_latch.h"
#include "generic/utility.h"
#include "interface/memoryInterface.h"
#include "memory/memory_checks.h"

#pragma STDC FENV_ACCESS ON

ME::ME(EX_ME_latch* exme, ME_XC_latch* mexc)
{
    memoryInterface* MEM_Interface=NULL;
    exme_latch = exme;
    mexc_latch = mexc;
}

void ME::perform()
{
    if (exme_latch->isME_Enabled == false)
    {
        xout<<"ME STALL\n";
        return;
    }
    else
    {
        xout<<"ME start\n";
    }

    //###start
    mexc_latch->rw_pack = exme_latch->rw_pack;
    mexc_latch->xc_pack = exme_latch->xc_pack;

    instruction* ins = &exme_latch->Ins;
    addrType memoryAddress = exme_latch->m_addr;
    int returnValue = RET_SUCCESS;

    if(exme_latch->isMemIns == true )
    {
        if( exme_latch->isLoad == true)
        {
            // xout<<"zzzz\n";
            
            memoryMessage newMessage1 (this, MemoryRead,
                    exme_latch->m_addr, 0, 
                    0, 0, 4, 
                    clock_cycles+MEM_LAT, 41);

            memoryMessage newMessage2 (this, MemoryRead,
                    exme_latch->m_addr+4, 0,
                    0, 0, 4, 
                    clock_cycles+MEM_LAT, 42);

            MEM_Interface->pendingMemoryAccessRequests.push(newMessage1);
            MEM_Interface->pendingMemoryAccessRequests.push(newMessage2);

            exme_latch->countReq=0;
            exme_latch->nReq = 2;
        }
        else if (exme_latch->isInt == true && 
                 exme_latch->isStore == true) ///StoreIntegerInstructions
        {
            xout << "Integer Store" << ins->op3 << "\n";
            regType regRD = exme_latch->regRD;
            regType regNextRD = exme_latch->regNextRD;

            xout << "Memory Address "<< memoryAddress << endl;
            
            if(ins->op3 == 5)
            {
                //STB opcode:000101
                char byte = regRD & 0x000000FF;

                memoryMessage newMessage (this, MemoryWrite,
                        exme_latch->m_addr, byte, 
                        0 ,0, 1, 
                        clock_cycles+MEM_LAT, 441);

                MEM_Interface->pendingMemoryAccessRequests.push(newMessage);
                exme_latch->nReq += 1;
            }
            else if (ins->op3 == 6)
            {
                //STH opcode:000110
                short halfWord = regRD & 0x0000FFFF;
                if(is_mem_address_not_aligned(memoryAddress, HALFWORD_ALIGN))
                {
                    xout << ("Source memory address not half word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }
                
                memoryMessage newMessage (this, MemoryWrite,
                        exme_latch->m_addr, 0, 
                        halfWord, 0, 4, 
                        clock_cycles+MEM_LAT, 441);
                
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage);
                
                exme_latch->nReq += 1;
            }
            else if(ins->op3 ==4) 
            {
                //ST opcode:000100
                if (is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
                {
                    xout << ("Destination memory address not word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }
                
                xout<<"OP3-4 "<<memoryAddress<<" "<<regRD<<"\n";
                
                memoryMessage newMessage (this, MemoryWrite,
                        exme_latch->m_addr, 0, 
                        0,regRD, 8, 
                        clock_cycles+MEM_LAT, 441);
                
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage);
                exme_latch->nReq += 1;
            }
            else if(ins->op3 == 7) 
            {
                //STD opcode:000111
                if(is_register_mis_aligned(ins->rd))
                {
                    // An integer load instruction
                    xout << ("Destination is an odd-even register pair") << endl;
                    returnValue = illegal_instruction;
                }
                if(is_mem_address_not_aligned(memoryAddress, DOUBLEWORD_ALIGN))
                {
                    xout << ("Destination memory address not double word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }

                memoryMessage newMessage1 (this, MemoryWrite,
                        exme_latch->m_addr, 0, 
                        0, regRD, 8, 
                        clock_cycles+MEM_LAT, 441);

                memoryMessage newMessage2 (this, MemoryWrite,
                        exme_latch->m_addr+4, 0, 
                        0, regNextRD, 8, 
                        clock_cycles+MEM_LAT, 441); 

                MEM_Interface->pendingMemoryAccessRequests.push(newMessage1);
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage2);
                exme_latch->nReq += 2;
            }
        
            returnValue = RET_SUCCESS;
        }
        else if(exme_latch->isFloat == true && exme_latch->isStore == true) ///StoreFloatInstructions
        { 
            regType regRD = exme_latch->regRD;
            regType regNextRD = exme_latch->regNextRD;
            if(ins->op3 == 36)
            {
                //STF opcode:100100
                memoryMessage newMessage (this, MemoryWrite,
                        exme_latch->m_addr, 0, 
                        0, regRD, 8, 
                        clock_cycles+MEM_LAT, 441);
                
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage);
                exme_latch->nReq += 1;
            }
            else if(ins->op3 == 39)
            {
                //STDF opcode:100111
                if(is_register_mis_aligned(ins->rd))
                {
                    xout << ("Source is an odd-even register pair") << endl;
                    int t_fsr = exme_latch->FSR;
                    t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
                    mexc_latch->rw_pack.FSR = t_fsr;
                    mexc_latch->rw_pack.isFSR = true;
                    returnValue = fp_exception;
                }

                if(is_mem_address_not_aligned(memoryAddress, DOUBLEWORD_ALIGN))
                {
                    xout << ("Source memory address double word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }

                memoryMessage newMessage1 (this, MemoryWrite,
                        exme_latch->m_addr, 0, 
                        0, regRD, 8, 
                        clock_cycles+MEM_LAT, 441);
                            
                memoryMessage newMessage2 (this, MemoryWrite,
                        exme_latch->m_addr+4, 0, 
                        0,regNextRD, 8, 
                        clock_cycles+MEM_LAT, 441);
                
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage1);
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage2);
                exme_latch->nReq += 2;
            }
            else if(ins->op3 ==38)
            {
                //STDFQ opcode:100110                            
                xout << "FP EXCEPTION TRAP" << endl;
                int t_fsr = exme_latch->FSR;
                t_fsr = modifyBits(4, t_fsr, FSR_ftt_l, FSR_ftt_r);
                mexc_latch->rw_pack.FSR = t_fsr;
                mexc_latch->rw_pack.isFSR = true;

                returnValue = fp_exception;
            }
            else if(ins->op3 == 37)
            {
                //STFSR opcode:100101
                int fsr = exme_latch->FSR;   
                memoryMessage newMessage (this, MemoryWrite,
                        exme_latch->m_addr, 0, 
                        0, fsr, 8, 
                        clock_cycles+MEM_LAT, 441);

                MEM_Interface->pendingMemoryAccessRequests.push(newMessage);
                exme_latch->nReq += 1;
                
                int t_fsr = exme_latch->FSR;
                t_fsr = modifyBits(0, t_fsr, FSR_ftt_l, FSR_ftt_r);
                mexc_latch->rw_pack.FSR = t_fsr;
                mexc_latch->rw_pack.isFSR = true;
            }

            returnValue = RET_SUCCESS;
        }
        else if(exme_latch->isAtomic == true)   //// Atomic Operations
        {   
            memoryMessage newMessage1 (this, MemoryRead,
                    exme_latch->m_addr, 0, 
                    0, 0, 1, 
                    clock_cycles+MEM_LAT, 40);
            
            MEM_Interface->pendingMemoryAccessRequests.push(newMessage1);
            exme_latch->nReq += 1;                         

            if(ins->op3==13)
            {
                //LDSTUB OP3=001101
                memoryMessage newMessage (this, MemoryWrite,
                        exme_latch->m_addr, 0xFF, 
                        0, 0, 1, 
                        clock_cycles+MEM_LAT, 441);
                
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage);
                exme_latch->nReq += 1;
                returnValue = RET_SUCCESS;
            }
            // else if(ins->op3==29){
            //     //LDSTUBA OP3=011101
            //     xout << "LDSTUBA Trap, No Alternate space" << endl;
            //     returnValue = privileged_instruction;
            // }                         
            
            returnValue = RET_QUIT;
        }
        else if (exme_latch->isSwap == true)
        {
            //// SWAP Operations
            regType regRD = exme_latch->regRD;
            if(ins->op3==15)
            {
                //SWAP opcode:001111
                if (is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
                {
                    xout << ("Destination memory address not word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }   

                memoryMessage newMessage1 (this, MemoryRead,
                        exme_latch->m_addr, 0, 
                        0, 0, 8, 
                        clock_cycles+MEM_LAT, 43);
                
                MEM_Interface->pendingMemoryAccessRequests.push(newMessage1);
                exme_latch->nReq += 1;
 
                memoryMessage newMessage2 (this, MemoryWrite,
                        exme_latch->m_addr, 0, 
                        0, regRD, 8, 
                        clock_cycles+MEM_LAT, 441);

                MEM_Interface->pendingMemoryAccessRequests.push(newMessage2);
                exme_latch->nReq += 1;                       
            }                      

            returnValue = RET_SUCCESS;
        }
        // else
        // { 
        //      /// until all RW complete, 
        //      //  Use :to check that #Cycles are multiples 
        //      //  of original 0 latency cycles (without interlocks)
        //      
        //      xout<<"dummy\n";
        //      memoryMessage dummy (this, MemoryRead,
        //              exme_latch->m_addr+4, 0, 
        //              0, 0, 4, 
        //              clock_cycles+MEM_LAT, 432);
        //
        //      MEM_Interface->pendingMemoryAccessRequests.push(dummy);
        //      exme_latch->countReq=0;
        //      exme_latch->nReq = 1;
        //  }
    }
    else
    { 
        // Not a memory instruction
        exme_latch->isME_Enabled = false;
        mexc_latch->isXC_Enabled = true;
        mexc_latch->retVal = exme_latch->retVal;
        return;
    }

    exme_latch->isME_Enabled = false;
}

void ME::handleEvent() 
{
    int returnValue = RET_SUCCESS;
    
    xout<<"THEREQID"<<MEM_Interface->reqID<<"\n";
    char* halfword0;
    char* halfword4;
    unsigned long  word0;

    switch (MEM_Interface->reqID)
    {
        case 40:
            exme_latch->byte = MEM_Interface->read_byte;
            break;
        case 41:    // Memory Read 32 bit at +0 offset
            exme_latch->halfword0 = MEM_Interface->read_halfword;
            break;
        case 42:    // Memory Read 32 bit at +4 offset
            exme_latch->halfword4 = MEM_Interface->read_halfword;
            break;
        case 43:    // Memory Read 64 bit
            exme_latch->word0 = MEM_Interface->read_word;
            break;
        case 441:   // Memory Write
            returnValue = MEM_Interface->write_status;
            if (returnValue != RET_SUCCESS)
            {
                xout<<"MEM WRITE ERROR\n";
            }
            break;
        default:
            xout<<"ME: UNKONWN RESPONSE FROM MEM INTERFACE\n";
    }
    
    exme_latch->nReq -= 1;
    if(exme_latch->nReq != 0)
    { 
        // All requests not complete
        return;
    }

    // Assigning from latch to local
    halfword0 = exme_latch->halfword0;
    halfword4 = exme_latch->halfword4;
    word0 = exme_latch->word0;

///////////////////////Handling...
    instruction* ins = &exme_latch->Ins;
    addrType memoryAddress = exme_latch->m_addr;
    returnValue = RET_SUCCESS;

    ///regRD and regNextRD below each
    if(exme_latch->isMemIns == true)
    { 
        xout<<"TMEMINS\n";
        if (exme_latch->isLoad == true && 
            exme_latch->isInt == true )
        { 
            ///LoadIntegerInstructions
            xout<<"LINT\n";
            xout<<"MADDR_PERF "<<memoryAddress<<"\n";
            
            int word = 0, hexDigit;
            char* dataWord;
            dataWord = halfword0;
            
            if(ins->op3 == 9)
            {
                //LDSB opcode:001001
                word = dataWord[0];
                mexc_latch->rw_pack.RD = ins->rs1;
                mexc_latch->rw_pack.RD_val = word;
                mexc_latch->rw_pack.isIntReg = true;                    
            }                
            if(ins->op3 ==  10)
            {
                //LDSH opcode:001010
                if(is_mem_address_not_aligned(memoryAddress, HALFWORD_ALIGN))
                {
                    xout << ("Source memory address not half word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }

                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                
                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                if(getBit(word, 15) == 1)
                    word = word | 0xFFFF0000;

                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = word;
                mexc_latch->rw_pack.isIntReg = true;                       
            }
                               
            if(ins->op3 == 1)
            {
                //LDUB opcode:000001
                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = word;
                mexc_latch->rw_pack.isIntReg = true;
            }                  
            if(ins->op3 == 2)
            {
                //LDUH opcode:000010
                if(is_mem_address_not_aligned(memoryAddress, HALFWORD_ALIGN))
                {
                    xout << ("Source memory address not half word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }

                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = word;
                mexc_latch->rw_pack.isIntReg = true;                        
            }
            
            if(ins->op3 == 0)
            {
                //LD opcode:000000
                if(is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
                {
                    xout << ("Source memory address not word aligned") <<endl;
                    returnValue = mem_address_not_aligned;
                }

                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[2]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[3]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                
                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = word;
                mexc_latch->rw_pack.isIntReg = true;
            }    
            if(ins->op3 == 3)
            {
                //LDD opcode:000011
                if(is_register_mis_aligned(ins->rd))
                {
                    xout << ("Destination is an odd-even register pair:illegal_instruction") <<endl;
                    returnValue = illegal_instruction;
                }                      

                if(is_mem_address_not_aligned(memoryAddress, DOUBLEWORD_ALIGN))
                {
                    xout << ("Source memory address not double word aligned")<<endl;
                    returnValue = mem_address_not_aligned;
                }

                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                
                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[2]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[3]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = word;
                mexc_latch->rw_pack.isIntReg = true;                      

                dataWord = halfword4;

                word = 0;
                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                
                hexDigit = dataWord[2]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24;
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[3]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = word;
                mexc_latch->rw_pack.isIntReg = true;
            }                   

            delete [] (dataWord);
            returnValue = RET_SUCCESS;
        }
        else if(exme_latch->isLoad == true && 
                exme_latch->isFloat == true ) ///LoadFloatingPointInstructions
        {
            int word=0, hexDigit;
            char* dataWord;
            dataWord = halfword0;

            if(ins->op3 == 32)
            {
                //LDF opcode:100000                  
                if(is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
                {
                    xout <<("Source memory address not word aligned")<<endl;
                    returnValue = mem_address_not_aligned;
                }
                                       
                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                
                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[2]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[3]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                mexc_latch->rw_pack.F_nWords = 1;
                mexc_latch->rw_pack.F_RD1 = ins->rd;
                mexc_latch->rw_pack.F_RD1_val = word;
                mexc_latch->rw_pack.isFltReg = true;
            }                  
            else if(ins->op3 == 35)
            {
                //LDDF opcode:100011
                if(is_register_mis_aligned(ins->rd))
                {
                    // An integer load instruction
                    xout << ("Destination is an odd-even register pair") <<endl;
                    int t_fsr = exme_latch->FSR;
                    t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
                    mexc_latch->rw_pack.FSR = t_fsr;
                    mexc_latch->rw_pack.isFSR = true;                          
                    returnValue = fp_exception;
                }

                if(is_mem_address_not_aligned(memoryAddress, DOUBLEWORD_ALIGN))
                {
                    xout << ("Source memory address not double word aligned")<<endl;
                    returnValue = mem_address_not_aligned;
                }
                        
                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[2]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[3]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                mexc_latch->rw_pack.F_RD1 = ins->rd;
                mexc_latch->rw_pack.F_RD1_val = word;

                dataWord = halfword4;
                word = 0;

                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[2]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24;
                word = (word << 8) | hexDigit;
                
                hexDigit = dataWord[3]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                
                mexc_latch->rw_pack.F_RD1 = ins->rd+1;
                mexc_latch->rw_pack.F_RD1_val = word;
                mexc_latch->rw_pack.F_nWords = 2;
                mexc_latch->rw_pack.isFltReg = true;                    
            }
            else if(ins->op3 == 33)
            {
                //LDFSR opcode:100001
                if(is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
                {
                    xout << ("Source memory address not word aligned") << endl;
                    returnValue = mem_address_not_aligned;
                }

                word = 0;
                hexDigit = dataWord[0]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;
                
                hexDigit = dataWord[1]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[2]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                hexDigit = dataWord[3]; 
                hexDigit = ((unsigned int)hexDigit << 24) >> 24; 
                word = (word << 8) | hexDigit;

                mexc_latch->rw_pack.FSR = word;
                mexc_latch->rw_pack.isFSR = true;
            }                   

            delete [] (dataWord);
            returnValue = RET_SUCCESS;
        }
        else if(exme_latch->isAtomic == true)
        {      
            //// Atomic Operations
            char byte = exme_latch->byte;                      
            if(ins->op3==13)
            {
                //LDSTUB OP3=001101
                regType regRD = ((unsigned int)(((unsigned int)byte)<<24)>>24);
                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = regRD;
                mexc_latch->rw_pack.isIntReg = true;
                returnValue = RET_SUCCESS;
            }                        
            else if(ins->op3==29)
            {
                //LDSTUBA OP3=011101
                xout << "LDSTUBA Trap, No Alternate space" << endl;
                returnValue = privileged_instruction;
            }           
            returnValue = RET_QUIT;
        }
        else if(exme_latch->isSwap == true)
        {        
            //// SWAP Operations
            if(ins->op3==15)
            {
                mexc_latch->rw_pack.RD = ins->rd;
                mexc_latch->rw_pack.RD_val = word0;
                mexc_latch->rw_pack.isIntReg = true;               
            }                        
            returnValue = RET_SUCCESS;
        }
    }
    
    mexc_latch->retVal = exme_latch->retVal; 
    mexc_latch->Ins = *ins;
    
    if(exme_latch->isMemIns == true)
    {
        mexc_latch->retVal = returnValue;
    }
    
    if(exme_latch->countReq >= exme_latch->nReq)
    { 
        /// Requirement Already satisfied(above), to remove cond. later
        exme_latch->isME_Enabled = false;
        mexc_latch->isXC_Enabled = true;
        bzero(exme_latch, sizeof(EX_ME_latch));
    }
/////////////////////////////////Handling
}


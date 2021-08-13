#include <math.h>
#include <cfenv> 
#include <iostream>
#include <iomanip>
#include <strings.h>
#include"EX.h"
#include "generic/utility.h"
#include "generic/header.h"
#include "memory/memory_checks.h"

#pragma STDC FENV_ACCESS ON

EX::EX( RA_EX_latch* raex, EX_ME_latch* exme )
{
    raex_latch = raex;
    exme_latch = exme;    
}

void EX::handleEvent()
{
}

inline int EX::addOverflow(regType regRS1, regType reg_or_imm, regType regRD)
{
    int signBit_regRS1, signBit_reg_or_imm, signBit_regRD;
	
	signBit_regRS1 = getBit(regRS1, SIGN_BIT);
	signBit_reg_or_imm = getBit(reg_or_imm, SIGN_BIT);
	signBit_regRD = getBit(regRD, SIGN_BIT);

    if ((signBit_regRS1==1 && signBit_reg_or_imm==1 && signBit_regRD==0) || 
        (signBit_regRS1==0 && signBit_reg_or_imm==0 && signBit_regRD==1)) 
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}

inline int EX::subtractOverflow(regType regRS1, regType reg_or_imm, regType regRD)
{
    int signBit_regRS1, signBit_reg_or_imm, signBit_regRD;
	
	signBit_regRS1 = getBit(regRS1, SIGN_BIT);
	signBit_reg_or_imm = getBit(reg_or_imm, SIGN_BIT);
	signBit_regRD = getBit(regRD, SIGN_BIT);

    if ((signBit_regRS1==1 && 
         (signBit_reg_or_imm==0 && 
          signBit_regRD==0)) || 
        (signBit_regRS1==0 && 
         (signBit_reg_or_imm==1 && signBit_regRD==1)))
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}

inline int EX::addCarry(regType regRS1, regType reg_or_imm, regType regRD)
{
    int signBit_regRS1, signBit_reg_or_imm, signBit_regRD;
	
	signBit_regRS1 = getBit(regRS1, SIGN_BIT);
	signBit_reg_or_imm = getBit(reg_or_imm, SIGN_BIT);
	signBit_regRD = getBit(regRD, SIGN_BIT);

    if ( (signBit_regRS1==1 && signBit_reg_or_imm==1) || 
         (signBit_regRD==0 && 
          (signBit_regRS1==1 || signBit_reg_or_imm==1)) )  
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}

inline int EX::subtractCarry(regType regRS1, regType reg_or_imm, regType regRD)
{
    int signBit_regRS1, signBit_reg_or_imm, signBit_regRD;
	
	signBit_regRS1 = getBit(regRS1, SIGN_BIT);
	signBit_reg_or_imm = getBit(reg_or_imm, SIGN_BIT);
	signBit_regRD = getBit(regRD, SIGN_BIT);

    if ((signBit_regRS1==0 && signBit_reg_or_imm==1) || 
        (signBit_regRD==1 && 
         (signBit_regRS1==0 || signBit_reg_or_imm==1)))
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}

int EX::LoadIntegerInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;

    addrType memoryAddress = 
        ((unsigned int)regRS1) + imm_or_reg;

    exme_latch->m_addr = memoryAddress;
    exme_latch->isLoad = true;
    exme_latch->isInt = true;
    exme_latch->isMemIns = true;
   
    return RET_SUCCESS;
}

int EX::LoadFloatingPointInstructions(instruction* ins)
{
    if(raex_latch->l_PSR_EF == 0)
    {
        xout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }

    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    memAddress memoryAddress = 
        ((unsigned int)regRS1) + imm_or_reg;

    exme_latch->m_addr = memoryAddress;
    exme_latch->isLoad = true;
    exme_latch->isFloat = true;
    exme_latch->isMemIns = true;

    return RET_SUCCESS;
}

int EX::StoreIntegerInstructions(instruction* ins)
{
    regType regRD = raex_latch->l_regRD;

    regType imm_or_reg, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    
    memAddress memoryAddress = 
        (unsigned int)regRS1 + imm_or_reg; 
    
    exme_latch->m_addr = memoryAddress;
    exme_latch->isStore = true;
    exme_latch->isInt = true;
    exme_latch->regRD = raex_latch->l_regRD;
    exme_latch->regNextRD = raex_latch->l_regNextRD;
    exme_latch->isMemIns = true;
    
    return RET_SUCCESS;
}

int EX::StoreFloatInstructions(instruction* ins)
{
    if(raex_latch->l_PSR_EF==0)
    {
        xout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }
  
 	regType regRD = raex_latch->l_regRD;
    regType imm_or_reg, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    memAddress memoryAddress = 
        (unsigned int) regRS1 + imm_or_reg;

    exme_latch->m_addr = memoryAddress;
    exme_latch->isStore = true;
    exme_latch->isFloat = true;
    exme_latch->regRD = raex_latch->l_regRD;
    exme_latch->regNextRD = raex_latch->l_regNextRD;
    exme_latch->FSR = raex_latch->l_FSR;
    exme_latch->isMemIns = true;

    return RET_SUCCESS;
}

int EX::AtomicLoadStoreUnsignedByte(instruction* ins)
{
    
    regType imm_or_reg, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;

    memAddress memoryAddress =
        (unsigned int)regRS1 + imm_or_reg;

    exme_latch->m_addr = memoryAddress;
    exme_latch->isAtomic = true;
    exme_latch->isMemIns = true;


    return RET_SUCCESS;
}

int EX::SWAP(instruction* ins)
{

    regType regRD;
    regType imm_or_reg, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    memAddress memoryAddress = 
        (unsigned int)regRS1 + imm_or_reg;
    
    exme_latch->m_addr = memoryAddress;
    exme_latch->isSwap = true;
    exme_latch->regRD = raex_latch->l_regRD;
    exme_latch->isMemIns = true;
    
    return RET_SUCCESS;

}

void EX::SethiNop(instruction* ins)
{
    if((ins->rd == 0) && (ins->imm22 == 0))
    {
        //nop
    }
    else
    {
        //sethi
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.isIntReg = true;
        exme_latch->rw_pack.RD_val = 
            ((ins->imm22)<<10);
    }
}

void EX::LogicalInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    if(ins->op3 == 1)
    {
        //AND opcode:000001
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            (regRS1 & imm_or_reg);
    }
    else if(ins->op3 == 17)
    {
        //ANDcc opcode:010001
        result = regRS1 & imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        updateICCMulLogical(result);
    }
    else if(ins->op3 == 5)
    {
        //ANDN opcode:000101
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            regRS1 & (~imm_or_reg);
    }
    else if(ins->op3 == 21)
    {
        //ANDNcc opcode:010101
        result = regRS1 & (~imm_or_reg);
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        updateICCMulLogical(result);
    }
    else if(ins->op3 == 2)
    {
        //OR opcode:000010
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            regRS1 | imm_or_reg;
    }
    else if(ins->op3 == 18)
    {
        //ORcc opcode:010010
        result = regRS1 | imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        updateICCMulLogical(result);
    }
    else if(ins->op3 == 6)
    {
        //ORN opcode:000110
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            regRS1 | (~imm_or_reg);
    }
    else if(ins->op3 == 22)
    {
        //ORNcc opcode:010110
        result = regRS1 | (~imm_or_reg);
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        updateICCMulLogical(result);
    }
    else if(ins->op3 == 3)
    {
        //XOR opcode:000011
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            regRS1 ^ imm_or_reg;
    }
    else if(ins->op3 == 19)
    {
        //XORcc opcode:010011
        result = regRS1 ^ imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        updateICCMulLogical(result);
    }
    else if(ins->op3 == 7)
    {
        //XNOR opcode:000111
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = ~(regRS1 ^ (imm_or_reg));
    }
    else if(ins->op3 == 23)
    {
        //XNORcc opcode:010111
        result = ~(regRS1 ^ (imm_or_reg));
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        updateICCMulLogical(result);
    }
    exme_latch->rw_pack.isIntReg = true;
}

void EX::ShiftInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    imm_or_reg = imm_or_reg & 0x0000001F;

    if(ins->op3 == 37)
    {
        //SLL opcode:100101
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            regRS1 << imm_or_reg;
    }
    else if(ins->op3 == 38)
    {
        //SRL opcode:100110
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            ((unsigned int)regRS1) >> imm_or_reg;
    }
    else if(ins->op3 == 39)
    {
        //SRA opcode:100111
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 
            ((int)regRS1 >> imm_or_reg);
    }
    exme_latch->rw_pack.isIntReg = true;
}

void EX::AddInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;

    if(ins->op3==0)
    { 
        //ADD opcode:000000
        result=regRS1+imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
    }
    else if(ins->op3==16)
    {
        // ADDcc opcode:010000
        result=regRS1+imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        updateICCAddSubtract(regRS1, imm_or_reg, result,0);                   
    }
    else if(ins->op3==8)
    {
        // ADDX opcode:001000
        int carry = raex_latch->l_PSR_icc_C;
          
        result=regRS1 + imm_or_reg + carry;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
    }
    else if(ins->op3==24)
    {
        // ADDXcc opcode:011000
        int carry = raex_latch->l_PSR_icc_C;

        result=regRS1 + imm_or_reg + carry;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;

        updateICCAddSubtract(regRS1, imm_or_reg, result,0);
    }
    exme_latch->rw_pack.isIntReg = true;
}

int EX::TaggedAddInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    if(ins->op3==32)
    {
        // TADDcc opcode:100000
        result=regRS1 + imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        taggedAddSubtract(regRS1, imm_or_reg, result, 0, 0);
    }
    else if(ins->op3==34)
    {
        // TADDccTV opcode:100010
        result=regRS1 + imm_or_reg;
        if(taggedAddSubtract(regRS1, imm_or_reg, result, 1, 0)==RET_TRAP)
        {
            xout << "Tag overflow has occurred\n";
            return tag_overflow;
        }
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        
    }
    exme_latch->rw_pack.isIntReg = true;
    return RET_SUCCESS;
}

void EX::SubtractInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    if(ins->op3==4)
    { 
        //SUB opcode:000100
        result=regRS1 - imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
    }
    else if(ins->op3==20)
    {
        // SUBcc opcode:010100
        result=regRS1 - imm_or_reg;
 
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;

        updateICCAddSubtract(regRS1, imm_or_reg, result,1);
    }
    else if(ins->op3==12)
    {
        // SUBX opcode:001100
        int carry = raex_latch->l_PSR_icc_C;

        result=regRS1 - imm_or_reg - carry;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
    }
    else if(ins->op3==28)
    {
        // SUBXcc opcode:011100
        int carry = raex_latch->l_PSR_icc_C;

        result=regRS1 - imm_or_reg - carry;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;

        updateICCAddSubtract(regRS1, imm_or_reg, result,1);  
    }
    exme_latch->rw_pack.isIntReg = true;
}

int EX::TaggedSubtractInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    if(ins->op3==33)
    {
        // TSUBcc opcode:100001
        result=regRS1 - imm_or_reg;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        taggedAddSubtract(regRS1, imm_or_reg, result, 0, 1);
    }
    else if(ins->op3==35)
    {
        // TSUBccTV opcode:100011
        result=regRS1 - imm_or_reg;
        if(taggedAddSubtract(regRS1, imm_or_reg, result, 1, 1)==RET_TRAP)
        {
            xout << ("Tag overflow has occurred") << endl;
            return tag_overflow;
        }
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
    }
    exme_latch->rw_pack.isIntReg = true;
    return RET_SUCCESS;

}
 
void EX::MultiplyStepInstruction(instruction* ins)
{//SOME CONFUSION IN STEP 3
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    if(ins->op3 == 36){
        //MULScc opcode:100100
        
        regType y = raex_latch->l_Y;
        
        int lsb_regRS1 = getBit(regRS1,0);
        
        int sign_bit = (raex_latch->l_PSR_icc_N) ^ (raex_latch->l_PSR_icc_V);

        sign_bit = sign_bit & 0x00000001;

        regRS1= regRS1>>1;
        regRS1 = modifyBit(regRS1, 31, sign_bit);
        
        if(getBit(y,0)==1){
            exme_latch->rw_pack.RD = ins->rd;
            exme_latch->rw_pack.RD_val = regRS1 + imm_or_reg;
            updateICCAddSubtract(regRS1, imm_or_reg, regRS1 + imm_or_reg , 0);
        }
        else{
            exme_latch->rw_pack.RD = ins->rd;
            exme_latch->rw_pack.RD_val = regRS1;
            updateICCAddSubtract(regRS1, 0, regRS1, 0);
        }
        exme_latch->rw_pack.isIntReg = true;


        y = y>>1;
        y = modifyBit(y, 31, lsb_regRS1);
        exme_latch->rw_pack.Y = y;
        exme_latch->rw_pack.isY = true;
    }
}

void EX::MultiplyInstructions(instruction* ins)
{
    
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    
    if(ins->op3 == 10){
        //UMUL opcode:001010
        unsigned long extended_regRD;
        regType regY =0;
        extended_regRD = (unsigned long)regRS1 * (unsigned long)imm_or_reg;
        
        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        exme_latch->rw_pack.Y = regY;
        exme_latch->rw_pack.isY = true;
        
        result = extended_regRD & 0x00000000FFFFFFFF;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        exme_latch->rw_pack.isIntReg = true;


    }
    
    else if(ins->op3==11){
        //SMUL opcode:001011
        signed long extended_regRD;
        regType regY = 0;
        extended_regRD = (signed long)regRS1 * (signed long)imm_or_reg;
        
        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        exme_latch->rw_pack.Y = regY;
        exme_latch->rw_pack.isY = true;
        
        result = extended_regRD & 0x00000000FFFFFFFF;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        exme_latch->rw_pack.isIntReg = true;
    }
    
    else if(ins->op3 == 26){
        //UMULcc opcode:011010
        unsigned long extended_regRD;
        regType regY = 0;
        extended_regRD = (unsigned long)regRS1 * (unsigned long)imm_or_reg;
        
        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        exme_latch->rw_pack.Y = regY;
        exme_latch->rw_pack.isY = true;
        
        result = extended_regRD & 0x00000000FFFFFFFF;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        exme_latch->rw_pack.isIntReg = true;
        
        updateICCMulLogical(result);

    }
    
    else if(ins->op3==27){
        //SMULcc opcode:011011
        signed long long extended_regRD;
        regType regY = 0;
        extended_regRD = (signed long)regRS1 * (signed long)imm_or_reg;
        
        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        exme_latch->rw_pack.Y = regY;
        exme_latch->rw_pack.isY = true;
        
        result = extended_regRD & 0x00000000FFFFFFFF;
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        exme_latch->rw_pack.isIntReg = true;
        
        updateICCMulLogical(result);
    }
}

int EX::DivideInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    
    if(imm_or_reg == 0)
    {
            xout << ("Attempt to divide by zero") << endl;
            return division_by_zero;
    }
    
    if(ins->op3 == 14){
        //UDIV opcode:001110
        unsigned long dividend;
        
        dividend = raex_latch->l_Y;

        dividend = (dividend << 32) | regRS1;
        
        result = (int)(dividend /((unsigned int) imm_or_reg));
        
        unsigned long result_overflow = dividend /((unsigned int) imm_or_reg);
        if(result_overflow > UINT_MAX){
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 0xFFFFFFFF;
        }
        else{
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        }
    exme_latch->rw_pack.isIntReg = true;
    }
    
    else if(ins->op3 == 30){
        //UDIVcc opcode:011110
        unsigned long dividend;

        
        dividend = raex_latch->l_Y;

        dividend = (dividend << 32) | regRS1;
        result = dividend /(unsigned int) imm_or_reg;
        
        unsigned long result_overflow = dividend /((unsigned int) imm_or_reg);
        if(result_overflow > UINT_MAX){
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = 0xFFFFFFFF;
            updateICCDiv(0xFFFFFFFF, 1);
        }
        else{
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
            updateICCDiv(result, 0);

        }
    exme_latch->rw_pack.isIntReg = true;
    }
    
    else if(ins->op3 == 15){
        //SDIV opcode:001111
        signed long dividend;
        dividend = raex_latch->l_Y;

        dividend = (dividend << 32) | regRS1; 
        result = dividend / imm_or_reg;
        
        signed long result_overflow = dividend / imm_or_reg;
        if(result_overflow > INT_MAX){
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = INT_MAX;
        }
        else if(result_overflow < (signed int)INT_MIN){
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = INT_MIN;
        }
        else{
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
        }
    exme_latch->rw_pack.isIntReg = true;
    }
    
    else if(ins->op3 == 31){
        //SDIVcc opcode:011111
        signed long dividend;
        
                
        dividend = raex_latch->l_Y;

        dividend = (dividend << 32) | regRS1; 
        result = dividend / imm_or_reg;
        signed long result_overflow = dividend / imm_or_reg;
        
        if(result_overflow > INT_MAX){
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = INT_MAX;
            updateICCDiv(INT_MAX, 1);
        }
        else if(result_overflow < INT_MIN){
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = INT_MIN;
            updateICCDiv(INT_MIN, 1);
        }
        else{
        exme_latch->rw_pack.RD = ins->rd;
        exme_latch->rw_pack.RD_val = result;
            updateICCDiv(result, 0);
        }
        
    exme_latch->rw_pack.isIntReg = true;

    }
    
    return RET_SUCCESS;
    
}

int EX::SaveAndRestoreInstructions(instruction* ins)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
     
    if(ins->op3==60){

        //save op:111100
        
        short nextCWP;

        nextCWP = (raex_latch->l_CWP - 1) % raex_latch->l_nWP;

        if(nextCWP<0){
            nextCWP = nextCWP + raex_latch->l_nWP;

        }
        // Test for window overflow.
 
        if(getBit(raex_latch->l_WIM, nextCWP)==1)
        {
            xout << ("Register window overflow has occurred") <<endl;
            return window_overflow;
        }
        // If window_overflow does not take place.
        else
        {
            int t_psr = raex_latch->l_PSR;
            t_psr = modifyBits(nextCWP, t_psr, PSR_CWP_l, PSR_CWP_r);
            exme_latch->rw_pack.PSR = t_psr;
            exme_latch->rw_pack.isPSR = true;

            exme_latch->rw_pack.RD = ins->rd;
            exme_latch->rw_pack.RD_val = ((unsigned int)regRS1) + imm_or_reg;
            exme_latch->rw_pack.isIntReg = true;
            
            return RET_SUCCESS;
        }	
    }
	
    else if(ins->op3==61)
	{
        
        //restore
        short nextCWP;

        nextCWP = (raex_latch->l_CWP + 1)%raex_latch->l_nWP;

        // Test for window underflow.

        if(getBit(raex_latch->l_WIM, nextCWP)==1)
        {
            //xout << nextCWP << ":nextCWP" << endl;
            xout << ("Register window underflow has occurred") << endl;
            return window_underflow;
        }
        // If window_underflow does not take place.
        else
        {
            
            int t_psr = raex_latch->l_PSR;
            t_psr = modifyBits(nextCWP, t_psr, PSR_CWP_l, PSR_CWP_r);
            exme_latch->rw_pack.PSR = t_psr;
            exme_latch->rw_pack.isPSR = true;

            exme_latch->rw_pack.RD = ins->rd;
            exme_latch->rw_pack.RD_val = (unsigned int)regRS1 + imm_or_reg;
            exme_latch->rw_pack.isIntReg = true;
            
            return RET_SUCCESS;
        }	
	}
    
    
}

void EX::BranchIntegerInstructions(instruction* ins)
{
    addrType regPC=raex_latch->l_PC;

    if(ins->cond==8){
         //BA cond:1000
        
        if(ins->a==1){
            exme_latch->rw_pack.PC = regPC+ins->disp22*4;
            exme_latch->rw_pack.nPC = regPC+ins->disp22*4+4;
            exme_latch->rw_pack.isCounter = true;
        }
        else{

            exme_latch->rw_pack.PC = raex_latch->l_nPC;
            exme_latch->rw_pack.nPC = regPC+ins->disp22*4;
            exme_latch->rw_pack.isCounter = true;
        }
    }
    else if(ins->cond==0){
         //BN cond:0000
        if(ins->a==1){
 
            exme_latch->rw_pack.PC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+8;
            exme_latch->rw_pack.isCounter = true;
        }
        else{

            exme_latch->rw_pack.PC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+8;
            exme_latch->rw_pack.isCounter = true;
        }
    }
    else{
        int condition=0;
        if(ins->cond==9) //BNE cond:1001
            condition = ~raex_latch->l_PSR_icc_Z;

        if(ins->cond==1) //BE cond:0001
            condition = raex_latch->l_PSR_icc_Z;

        if(ins->cond==10){ //BG cond:1010
            condition = ~(raex_latch->l_PSR_icc_Z | (raex_latch->l_PSR_icc_N ^ raex_latch->l_PSR_icc_V));

        }
        if(ins->cond==2) //BLE cond:0010
            condition = raex_latch->l_PSR_icc_Z | (raex_latch->l_PSR_icc_N ^ raex_latch->l_PSR_icc_V);

        if(ins->cond==11) //BGE cond:1011
            condition = ~(raex_latch->l_PSR_icc_N ^ raex_latch->l_PSR_icc_V);

        if(ins->cond==3) //BL cond:0011
        	condition = raex_latch->l_PSR_icc_N ^ raex_latch->l_PSR_icc_V;

        if(ins->cond==12)  //BGU cond:1100
        	condition = ~(raex_latch->l_PSR_icc_C | raex_latch->l_PSR_icc_Z);

        if(ins->cond==4) //BLEU cond:0100
        	condition = raex_latch->l_PSR_icc_C | raex_latch->l_PSR_icc_Z;

        if(ins->cond==13) //BCC cond:1101
        	condition = ~raex_latch->l_PSR_icc_C;

        if(ins->cond==5) //BCS cond:0101
            condition = raex_latch->l_PSR_icc_C;

        if(ins->cond==14) //BPOS cond:1110
            condition = ~raex_latch->l_PSR_icc_N;

        if(ins->cond==6) //BNEG cond:0110
        	condition = ~raex_latch->l_PSR_icc_N;

        if(ins->cond==15) //BVC cond:1111
        	condition = ~raex_latch->l_PSR_icc_V;

        if(ins->cond==7) //BVS cond:0111
        	condition = ~raex_latch->l_PSR_icc_V;

        condition = getBit(condition, 0);
        
        xout << "   REACHING ON BRANCH Condition "<< condition << endl;
        
        if(condition==1)
		{
				// Branch taken

            exme_latch->rw_pack.PC = raex_latch->l_nPC;
            exme_latch->rw_pack.nPC = regPC+ins->disp22*4;
            exme_latch->rw_pack.isCounter = true;
		}
        else{
            // Branch NOT taken
            if(ins->a==1){
                // Annul bit = 1

            exme_latch->rw_pack.PC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+8;
            exme_latch->rw_pack.isCounter = true; 
			nBT++;
            }
            else{

				// Annul bit = 0

            exme_latch->rw_pack.PC = raex_latch->l_nPC;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.isCounter = true; 
			
		    }
        }
        
	}
}

int EX::BranchFloatInstructions(instruction* ins)
{
   
    addrType regPC=raex_latch->l_PC;
    
    if(raex_latch->l_PSR_EF==0){
        xout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }

    if(ins->cond==8){
        //FBA instruction
        unsigned int next_pc = raex_latch->l_PC+4*((int)ins->disp22);//UNSIGNED INT REMOVED FROM HERE
        if(ins->a==1){

            exme_latch->rw_pack.PC = next_pc;
            exme_latch->rw_pack.nPC = next_pc+4;
            exme_latch->rw_pack.isCounter = true; 
        }
        else{
 
            exme_latch->rw_pack.PC = raex_latch->l_nPC;
            exme_latch->rw_pack.nPC = next_pc;
            exme_latch->rw_pack.isCounter = true; 
        }
    }
    
    else if(ins->cond==0){
        //FBN instruction
        if(ins->a==1){

            exme_latch->rw_pack.PC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+8;
            exme_latch->rw_pack.isCounter = true; 
        }
        else{

            exme_latch->rw_pack.PC = raex_latch->l_nPC;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.isCounter = true; 
        }
    }
    
    else{
        int condition = raex_latch->l_FSR_fcc;

        bool is_branch = false;
        if(ins->cond==7); //FBU cond:0111
            if(condition==3)
                is_branch=true;
        if(ins->cond==6) //FBG cond:0110
            if(condition==2)
                is_branch=true;
        if(ins->cond==5) //FBUG cond:0101
            if(condition==2 || condition==3)
                is_branch=true;
        if(ins->cond==4) //FBL cond:0100
            if(condition==1)
                is_branch=true;
        if(ins->cond==3) //FUBL cond:0011
            if(condition==1 || condition==3)
                is_branch=true;
        if(ins->cond==2) //FBLG cond:0010
            if(condition==1 || condition==2)
                is_branch=true;
        if(ins->cond==1)  //FBNE cond:0001
            if(condition!=0)
                is_branch=true;
        if(ins->cond==9) //FBE cond:1001
            if(condition==0)
                is_branch=true;
        if(ins->cond==10) //FBUE cond:1010
            if(condition==3 || condition==0)
                is_branch=true;
        
        if(ins->cond==11) //FBGE cond:1011
            if(condition==0 || condition==2)
                is_branch=true;
        
        if(ins->cond==12) //FBUGE cond:1100
            if(condition==3 || condition==2 || condition==0)
                is_branch=true;
        
        if(ins->cond==13) //FBLE cond:1101
            if(condition==1 || condition==0)
                is_branch=true;
        
        if(ins->cond==14) //FBULE cond:1110
            if(condition!=2)
                is_branch=true;
        
        if(ins->cond==15) //FBO cond:1111
            if(condition==0 || condition==1 || condition==2)
                is_branch=true;
        
        if(is_branch){

            exme_latch->rw_pack.PC = raex_latch->l_nPC;
            exme_latch->rw_pack.nPC = regPC+ins->disp22*4;
            exme_latch->rw_pack.isCounter = true; 
            nBT++;
		}
        
        else{
            // Branch NOT taken
            if(ins->a==1){

            exme_latch->rw_pack.PC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+8;
            exme_latch->rw_pack.isCounter = true; 
            }
            else{
                // Annul bit = 0

            exme_latch->rw_pack.PC = raex_latch->l_nPC;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.isCounter = true; 
            }
        }
    }

   return RET_SUCCESS; 
}

void EX::CallInstruction(instruction* ins)
{
    
    addrType regPC=raex_latch->l_PC;
    

    exme_latch->rw_pack.RD = 15;
    exme_latch->rw_pack.RD_val = regPC;
    exme_latch->rw_pack.isIntReg = true;
    
    exme_latch->rw_pack.PC = raex_latch->l_nPC;
    exme_latch->rw_pack.nPC = regPC+(int)(ins->disp30 << 2);
    exme_latch->rw_pack.isCounter = true;

    nBT++;

}

int EX::JumpAndLink(instruction* ins)
{
    
    regType imm_or_reg, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    
    memAddress memoryAddress = 
        (unsigned int)regRS1 + imm_or_reg;

    if(is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
    {
        xout << ("Destination memory address not word aligned") << endl;
        return mem_address_not_aligned;
    }
    
    exme_latch->rw_pack.RD = ins->rd;
    exme_latch->rw_pack.RD_val = raex_latch->l_PC;
    exme_latch->rw_pack.isIntReg = true;
    
    exme_latch->rw_pack.PC = raex_latch->l_nPC;
    exme_latch->rw_pack.nPC = memoryAddress;
    exme_latch->rw_pack.isCounter = true;
    
    nBT++;
    //xout << "JMPL: "<< sregister->getnPC()<<endl;
    
    return RET_SUCCESS;

}

int EX::RETT(instruction* ins)
{
        
    regType imm_or_reg, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    memAddress targetAddr = 
        (unsigned int)regRS1 + imm_or_reg;//UNSIGNED INT REMOVED FROM HERE
        
    short nextCWP;
    
    nextCWP = (raex_latch->l_CWP + 1)%raex_latch->l_nWP;
    
    // Traps:
    if(raex_latch->l_PSR_ET==1 && raex_latch->l_PSR_S==0){
        xout << "privileged_instruction trap"<<endl;
        return privileged_instruction;
    }
    if(raex_latch->l_PSR_ET==1 && raex_latch->l_PSR_S==1){
        xout << "illegal_instruction trap"<<endl;
        return illegal_instruction;
    }
    if(raex_latch->l_PSR_ET==0 && (raex_latch->l_PSR_S==0)){
        xout << "privileged_instruction trap"<<endl;
        return privileged_instruction;
    }
    if(raex_latch->l_PSR_ET==0 && getBit(raex_latch->l_WIM, nextCWP)==1){
        xout << "window_underflow trap"<<endl;
        return window_underflow;
    }
    if(raex_latch->l_PSR_ET==0 && is_mem_address_not_aligned(targetAddr,WORD_ALIGN)==1){
        xout << "mem_address_not_aligned trap"<<endl;
        return mem_address_not_aligned;
    }
    
    // If TRAPS does not take place.
    else
    {
    
        exme_latch->rw_pack.PC = raex_latch->l_nPC;
        exme_latch->rw_pack.nPC = targetAddr;
        exme_latch->rw_pack.isCounter = true;
        nBT++;
    
    
        int t_psr = raex_latch->l_PSR;
        t_psr = modifyBits(raex_latch->l_PSR_PS, t_psr, PSR_S_l, PSR_S_r);
        t_psr = modifyBits(1, t_psr, PSR_ET_l, PSR_ET_r);
        t_psr = modifyBits(nextCWP, t_psr, PSR_CWP_l, PSR_CWP_r);
        exme_latch->rw_pack.PSR = t_psr;
        exme_latch->rw_pack.isPSR = true;
    
    
        return RET_SUCCESS;
    }	 
}


int EX::TrapOnICC(instruction* ins)
{
    regType imm_or_reg, regRS1;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    
    int is_trap=0;
    if(ins->cond==8){
         //TA cond:1000
        is_trap=1;
    }
    if(ins->cond==0){
        //TN
        is_trap=0;
    }
    if(ins->cond==9){
        //TNE
        is_trap=~(raex_latch->l_PSR_icc_Z);
    }
    if(ins->cond==1){
        //TE
        is_trap=(raex_latch->l_PSR_icc_Z);
    }
    if(ins->cond==10){
        //TG
        is_trap=~(raex_latch->l_PSR_icc_Z|(raex_latch->l_PSR_icc_N^raex_latch->l_PSR_icc_V));
    }
    if(ins->cond==2){
        //TLE
        is_trap=(raex_latch->l_PSR_icc_Z|(raex_latch->l_PSR_icc_N^raex_latch->l_PSR_icc_V));
    }
    if(ins->cond==11){
        //TGE
        is_trap=~(raex_latch->l_PSR_icc_N^raex_latch->l_PSR_icc_V);
    }
    if(ins->cond==3){
        //TL
        is_trap=(raex_latch->l_PSR_icc_N^raex_latch->l_PSR_icc_V);
    }
    if(ins->cond==12){
        //TGU
        is_trap=~(raex_latch->l_PSR_icc_C|raex_latch->l_PSR_icc_Z);
    }
    if(ins->cond==4){
        //TLEU
        is_trap=(raex_latch->l_PSR_icc_C|raex_latch->l_PSR_icc_Z);
    }
    if(ins->cond==13){
        //TCC
        is_trap=~(raex_latch->l_PSR_icc_C);
    }
    if(ins->cond==5){
        //TCS
        is_trap=(raex_latch->l_PSR_icc_C);
    }
    if(ins->cond==14){
        //TPOS
        is_trap=~(raex_latch->l_PSR_icc_N);
    }
    if(ins->cond==6){
        //TNEG
        is_trap=~(raex_latch->l_PSR_icc_N);
    }
    if(ins->cond==15){
        //TVC
        is_trap=~(raex_latch->l_PSR_icc_V);
    }
    if(ins->cond==7){
        //TVS
        is_trap=(raex_latch->l_PSR_icc_V);
    }
    if(is_trap==1){
        int setVal = 128 + extract(regRS1+imm_or_reg,0,7);
        int t_tbr = raex_latch->l_TBR;
        t_tbr = modifyBits(setVal, t_tbr, TBR_TT_l, TBR_TT_r);
        exme_latch->rw_pack.TBR = setVal;
        exme_latch->rw_pack.isTBR = true;
        return setVal;
    }
    
    return RET_SUCCESS;
}

int EX::ReadStateRegisterInstructions(instruction* ins)
{
    int regRD = ins->rd;
    int op3 = ins->op3;
    int regRS1 = ins->rs1;
    
    if(op3==40 && regRS1==0){
        //RDY
        int y = raex_latch->l_Y;

        exme_latch->rw_pack.RD = regRD;
        exme_latch->rw_pack.RD_val = y;
        exme_latch->rw_pack.isIntReg = true;

    }
    else if(op3==40 && regRS1!=0){
        //RDASR AND STBAR
        int get_Val = raex_latch->l_ASR;

        exme_latch->rw_pack.RD = regRD;
        exme_latch->rw_pack.RD_val = get_Val;
        exme_latch->rw_pack.isIntReg = true;
    }
    else if(op3==41 && raex_latch->l_PSR_S==1){
        //RDPSR
        int get_Val = raex_latch->l_PSR;


        
        exme_latch->rw_pack.RD = regRD;
        exme_latch->rw_pack.RD_val = get_Val;
        exme_latch->rw_pack.isIntReg = true;
    }
    else if(op3==42 && raex_latch->l_PSR_S==1){
        //RDWIM
        int get_Val = raex_latch->l_WIM;


        exme_latch->rw_pack.RD = regRD;
        exme_latch->rw_pack.RD_val = get_Val;
        exme_latch->rw_pack.isIntReg = true;
    }
    else if(op3==43 && raex_latch->l_PSR_S==1){
        //RDTBR
        int get_Val = raex_latch->l_TBR;

        exme_latch->rw_pack.RD = regRD;
        exme_latch->rw_pack.RD_val = get_Val;
        exme_latch->rw_pack.isIntReg = true;
    }
    else{
        xout <<" Priviledged instruction trap " << endl;
        return privileged_instruction;
    }
    return RET_SUCCESS;
}

int EX::WriteStateRegisterInstructions(instruction* ins)
{
    regType imm_or_reg, regRS1, regRD;
    regRD = ins->rd;
    regRS1 = raex_latch->l_regRS1;
    imm_or_reg = raex_latch->l_reg_or_imm;
    int set_Val = 0;
    if(ins->op3==48 && regRD==0){
        //WRY
        set_Val=(imm_or_reg^regRS1);
        exme_latch->rw_pack.Y = set_Val;
        exme_latch->rw_pack.isY = true;
    }
    if(ins->op3==48 && regRD!=0){
        //WRASR
        set_Val=(imm_or_reg^regRS1);

        exme_latch->rw_pack.RD = regRD; /// checked no overlap with Int Regs
        exme_latch->rw_pack.RD_val = set_Val;
        exme_latch->rw_pack.isASR = true; 
        
    }
    else if (raex_latch->l_PSR_S==0){
        xout << "privileged_instruction" << endl;
        return privileged_instruction;
    }
    if(ins->op3==49 && raex_latch->l_PSR_S==1){
        //WRPSR
        set_Val=(imm_or_reg^regRS1);
        int tmp = (set_Val & 0x0000001F);
        
        if(getBit(raex_latch->l_WIM, tmp) >= (raex_latch->l_nWP)){
            xout << "illegal_instruction trap" << endl;
            return illegal_instruction;
        }
        exme_latch->rw_pack.PSR = set_Val;
        exme_latch->rw_pack.isPSR = true;
    }
    if(ins->op3==50 && raex_latch->l_PSR_S==1){
        //WRWIM
        
        set_Val=(imm_or_reg^regRS1);
        xout << "write wim=" << set_Val << endl;
        
        exme_latch->rw_pack.WIM = set_Val;
        exme_latch->rw_pack.isWIM = true;
    }
    if(ins->op3==51 && raex_latch->l_PSR_S==1){
        //WRTBR
        set_Val=(imm_or_reg^regRS1);
        exme_latch->rw_pack.TBR = set_Val;
        exme_latch->rw_pack.isTBR = true;
    }
    
    return RET_SUCCESS;
}

int EX::FpopInstructions(instruction* ins)
{
    
    int regFs2 = raex_latch->l_regFs2_1;
    
    if(raex_latch->l_PSR_EF==0){
        xout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }

    if(ins->opf==196 && ins->op3==52){
        //FiTOs
        float regRD;
        int Rd = raex_latch->l_FSR_RD;

        if(Rd==0){
            //nearest
            std::fesetround(FE_TONEAREST);
        }
        if(Rd==1){
            //0
            std::fesetround(FE_TOWARDZERO);
        }
        if(Rd==2){
            //+inf
            std::fesetround(FE_UPWARD);
        }
        if(Rd==3){
            //-inf
            std::fesetround(FE_DOWNWARD);
        }
        regRD = static_cast<float>(regFs2);
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = *((unsigned int *)&regRD);
        exme_latch->rw_pack.isFltReg = true;
    }
    
    if(ins->opf==200 && ins->op3==52){
        //FiTOd
        if(is_register_mis_aligned(ins->rd))
        {
            xout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }
        double regRD;
        int Rd = raex_latch->l_FSR_RD; // Rd - rounding direction

        if(Rd==0){
            //nearest
            std::fesetround(FE_TONEAREST);
        }
        if(Rd==1){
            //0
            std::fesetround(FE_TOWARDZERO);
        }
        if(Rd==2){
            //+inf
            std::fesetround(FE_UPWARD);
        }
        if(Rd==3){
            //-inf
            std::fesetround(FE_DOWNWARD);
        }
        regRD = static_cast<double>(regFs2);
        
        int l_32 = *((unsigned int *)(((unsigned int *)&regRD)+1));
        int h_32 = *((unsigned int* )&regRD);


        exme_latch->rw_pack.F_nWords = 2;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = h_32;
        exme_latch->rw_pack.F_RD2 = ins->rd+1;
        exme_latch->rw_pack.F_RD2_val = l_32;
        exme_latch->rw_pack.isFltReg == true;
    }
    
    if(ins->opf==204 && ins->op3==52){
        //FiTOq
        
        if((ins->rd)%4!=0)
        {
            xout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }
        
        int Rd = raex_latch->l_FSR_RD;

        if(Rd==0){
            //nearest
            std::fesetround(FE_TONEAREST);
        }
        if(Rd==1){
            //0
            std::fesetround(FE_TOWARDZERO);
        }
        if(Rd==2){
            //+inf
            std::fesetround(FE_UPWARD);
        }
        if(Rd==3){
            //-inf
            std::fesetround(FE_DOWNWARD);
        }
        long double regRD;
        regRD = static_cast<long double>(regFs2);
        
        unsigned int m_1 = *((unsigned int *)&regRD);
        unsigned int m_2 = *((unsigned int *)(((unsigned int *)&regRD)+1));
        unsigned int m_3 = *((unsigned int *)(((unsigned int *)&regRD)+2));
        unsigned int m_4 = *((unsigned int *)(((unsigned int *)&regRD)+3));
        

        exme_latch->rw_pack.F_nWords = 4;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = m_1;
        exme_latch->rw_pack.F_RD2 = ins->rd+1;
        exme_latch->rw_pack.F_RD2_val = m_2;
        exme_latch->rw_pack.F_RD3 = ins->rd+2;
        exme_latch->rw_pack.F_RD3_val = m_3;
        exme_latch->rw_pack.F_RD4 = ins->rd+3;
        exme_latch->rw_pack.F_RD4_val = m_4;
        exme_latch->rw_pack.isFltReg = true;
    }
    
    if(ins->opf==209 && ins->op3==52){
        //FsTOi
        float num = int_float(regFs2);
        int regD = num;
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = regD;
        exme_latch->rw_pack.isFltReg = true;
    }

    if(ins->opf==210 && ins->op3==52){
        //FdTOi
        if(is_register_mis_aligned(ins->rs2))
        {
            xout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }
        int regFs1_lsb = raex_latch->l_regFs2_2;
        int regFs1_msb = regFs2;
        long int regFS_tot = ((unsigned long int)regFs1_msb<<32) | ((unsigned int)regFs1_lsb);
        double num = lint_double(regFS_tot);
        xout << "num=" << num << " ,"<<regFS_tot << endl;
        xout << regFs1_msb << ", " << (unsigned int)regFs1_lsb << endl;
        int regD = num;
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = regD;
        exme_latch->rw_pack.isFltReg = true;
    }
    
    if(ins->opf==211 && ins->op3==52){
        //FqTOi --quad 128 to integer 32
        if((ins->rs2)%4!=0)
        {
            xout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }

        int regFs1_1 = raex_latch->l_regFs2_1;
        int regFs1_2 = raex_latch->l_regFs2_2;
        int regFs1_3 = raex_latch->l_regFs2_3;
        int regFs1_4 = raex_latch->l_regFs2_4;

        long int FS1_msb = (((long int)regFs1_1)<<32) | regFs1_2;
        long int FS1_lsb = (((long int)regFs1_3)<<32) | regFs1_4;
        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);
        int regD = num1;
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = regD;
        exme_latch->rw_pack.isFltReg = true;
    }
    
    if(ins->opf==201 && ins->op3==52){
        //FsTOd
        if(ins->rd%2!=0){
            xout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }
        float Fs2 = int_float(regFs2);
        double regR = Fs2;
        int msb = *(long int *)&regR;
        int lsb = *(((long int *)&regR)+1);

        exme_latch->rw_pack.F_nWords = 2;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = msb;
        exme_latch->rw_pack.F_RD2 = ins->rd+1;
        exme_latch->rw_pack.F_RD2_val = lsb;
        exme_latch->rw_pack.isFltReg = true;
        
    }
    
    if(ins->opf==205 && ins->op3==52){
        //FsTOq
        if(ins->rd%4!=0){
            xout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
            
            return fp_exception;
        }
        
        float Fs2 = int_float(regFs2);
        long double regR = Fs2;
        long unsigned int msb = *(long unsigned int *)&regR;
        long unsigned int lsb = *(((long unsigned int *)&regR)+1);


        exme_latch->rw_pack.F_nWords = 4;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = msb>>32;
        exme_latch->rw_pack.F_RD2 = ins->rd+1;
        exme_latch->rw_pack.F_RD2_val = msb;
        exme_latch->rw_pack.F_RD3 = ins->rd+2;
        exme_latch->rw_pack.F_RD3_val = lsb>>32;
        exme_latch->rw_pack.F_RD4 = ins->rd+3;
        exme_latch->rw_pack.F_RD4_val = lsb;
        exme_latch->rw_pack.isFltReg = true;
        
    }
    
    if(ins->opf==198 && ins->op3==52){
        //FdTOs
        if(ins->rs2%2!=0){
            xout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }

        int regFs1_msb = raex_latch->l_regFs2_1;            
        int regFs1_lsb = raex_latch->l_regFs2_2;

        long int FS1 =  (((unsigned long int)regFs1_msb)<<32) | ((unsigned long int)regFs1_lsb);        
        
        double num = lint_double(FS1);
        
        int Rd = raex_latch->l_FSR_RD;
        if(Rd==0){
            //nearest
            std::fesetround(FE_TONEAREST);
        }
        if(Rd==1){
            //0
            std::fesetround(FE_TOWARDZERO);
        }
        if(Rd==2){
            //+inf
            std::fesetround(FE_UPWARD);
        }
        if(Rd==3){
            //-inf
            std::fesetround(FE_DOWNWARD);
        }

        float regRD = static_cast<float>(num);
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = *((int *)&regRD);
        exme_latch->rw_pack.isFltReg = true;
    }
    
    if(ins->opf==206 && ins->op3==52){
        //FdTOq
        if(ins->rs2%2!=0 || ins->rd%4!=0){
            xout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
            
            return fp_exception;
        }

        int regFs1_msb = raex_latch->l_regFs2_1;            
        int regFs1_lsb = raex_latch->l_regFs2_2;

        long int FS1 =  (((unsigned long int)regFs1_msb)<<32) | ((unsigned long int)regFs1_lsb);        
        double num = lint_double(FS1);
        long double result = num;

        exme_latch->rw_pack.F_nWords = 4;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = *(int *)&result;
        exme_latch->rw_pack.F_RD2 = ins->rd+1;
        exme_latch->rw_pack.F_RD2_val = *(((int *)&result)+1);
        exme_latch->rw_pack.F_RD3 = ins->rd+2;
        exme_latch->rw_pack.F_RD3_val = *(((int *)&result)+2);
        exme_latch->rw_pack.F_RD4 = ins->rd+3;
        exme_latch->rw_pack.F_RD4_val = *(((int *)&result)+3);
        exme_latch->rw_pack.isFltReg = true;
    }
    
    if((ins->opf==199 || ins->opf==203) && ins->op3==52){
        //FqTOs, FqTOd
        if(ins->rs2%4!=0){
            xout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
            
            return fp_exception;
        }

        int regFs1_1 = raex_latch->l_regFs2_1;
        int regFs1_2 = raex_latch->l_regFs2_2;
        int regFs1_3 = raex_latch->l_regFs2_3;
        int regFs1_4 = raex_latch->l_regFs2_4;
        
        long int FS1_msb = ((unsigned long int)regFs1_1<<32) | regFs1_2;
        long int FS1_lsb = ((unsigned long int)regFs1_3<<32) | regFs1_4;
        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);
        
        int Rd = raex_latch->l_FSR_RD;

        if(Rd==0){
            //nearest
            std::fesetround(FE_TONEAREST);
        }
        if(Rd==1){
            //0
            std::fesetround(FE_TOWARDZERO);
        }
        if(Rd==2){
            //+inf
            std::fesetround(FE_UPWARD);
        }
        if(Rd==3){
            //-inf
            std::fesetround(FE_DOWNWARD);
        }
    
        if(ins->opf==203){

            if(ins->rd%2!=0){
                xout << ("Source is an odd-even register pair float quad instruction") << endl;
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;

                return fp_exception;
            }
            double regRD = static_cast<double>(num1);
 
            exme_latch->rw_pack.F_nWords = 2;
            exme_latch->rw_pack.F_RD1 = ins->rd;
            exme_latch->rw_pack.F_RD1_val = (*(int *)&regRD);
            exme_latch->rw_pack.F_RD2 = ins->rd+1;
            exme_latch->rw_pack.F_RD2_val = (*(((int *)&regRD)+1));
            exme_latch->rw_pack.isFltReg = true;
            
        
        }
        else{
            float regRD = static_cast<float>(num1);
            exme_latch->rw_pack.F_nWords = 1;
            exme_latch->rw_pack.F_RD1 = ins->rd;
            exme_latch->rw_pack.F_RD1_val = (*(int *)&regRD);
            exme_latch->rw_pack.isFltReg = true;

        }

    }
    
    
    if(ins->opf==1 && ins->op3==52){
        //FMOVs
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = regFs2;
        exme_latch->rw_pack.isFltReg = true;

    }
    
    if(ins->opf==5 && ins->op3==52){
        //FNEGs
        regFs2 = regFs2 ^ (0x80000000);
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = regFs2;
        exme_latch->rw_pack.isFltReg = true;

    }
    
    if(ins->opf==9 && ins->op3==52){
        //FABSs
        regFs2 = regFs2 & (0x7FFFFFFF);
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = regFs2;
        exme_latch->rw_pack.isFltReg = true;

    }
    
    if((ins->opf==65 || ins->opf==69 || ins->opf==77) && ins->op3==52){
        //FADDs, FSUBs, FDIVs

        int regFs1 = raex_latch->l_regFs1_1;
        regFs2 = raex_latch->l_regFs2_1;

        float num1 = int_float(regFs1);
        float num2 = int_float(regFs2);
        float result;
        
        if(ins->opf==65){
            result = num1 + num2;
        }
        
        if(ins->opf==69)
        {
            result = num1 - num2;       
        }
        
        if(ins->opf==77)
        {   
            if(num2 == 0){
                xout << "DIVIDE BY Zero or NaN or Infinity Exception" <<endl;
                return fp_exception;
            }
            result = num1 / num2;       
        }
        
        exme_latch->rw_pack.F_nWords = 1;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = *((int *)&result);
        exme_latch->rw_pack.isFltReg = true;
    }
    
    if((ins->opf==66 || ins->opf==70 || ins->opf==78) && ins->op3==52){
        //FADDd, FSUBd, FDIVd
        if(is_register_mis_aligned(ins->rs1)||is_register_mis_aligned(ins->rs2)||is_register_mis_aligned(ins->rd))
        {
            xout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        
            return fp_exception;
        }
        

        int regFs1_msb = raex_latch->l_regFs1_1;
        int regFs1_lsb = raex_latch->l_regFs1_2;
        
        long int FS1 = (((unsigned long int)regFs1_msb)<<32) | regFs1_lsb;


        int regFs2_msb = raex_latch->l_regFs2_1;
        int regFs2_lsb = raex_latch->l_regFs2_2;

        long int FS2 = (((unsigned long int)regFs2_msb)<<32) | regFs2_lsb;

        double num1 = lint_double(FS1);
        double num2 = lint_double(FS2);
        double result;
        if(ins->opf==66){
            xout << "num1 ="<<num1 << endl;
            xout << "num2 ="<<num2 << endl;
             
            result = num1 + num2;
            xout << "result="<<result << endl;
            xout << "result&d="<<&result <<endl;
            xout << "result&i="<<(int *)&result <<endl;
            xout << "result&int="<<*((int *)&result) <<endl;
            xout << "result&int+1="<<*((int *)&result+1) <<endl;
            xout << "result&int+1="<<(((*((long int *)&result)))>>32) <<endl;
             
        }
        if(ins->opf==70)
        {
             result = num1 - num2;       
        }
        if(ins->opf==78)
        {
            if(num2 == 0){
                xout << "DIVIDE BY ZERO" <<endl;
                return fp_exception;
            }
            result = num1 / num2;       
        }
        double result2 = result;
        xout << "result2=" << result2<<" ins->rd=" <<ins->rd<< endl;
 
        exme_latch->rw_pack.F_nWords = 2;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = *((unsigned long int *)&result2)>>32;
        exme_latch->rw_pack.F_RD2 = ins->rd+1;
        exme_latch->rw_pack.F_RD2_val = *(((unsigned long int *)&result2));
        exme_latch->rw_pack.isFltReg = true;
    }

    if((ins->opf==67 || ins->opf==71 || ins->opf==75 || ins->opf==79) && ins->op3==52){
        //FADDq, FSUBq, FMULq, FDIVq
        
        if((ins->rs1)%4!=0 || (ins->rs2)%4!=0 || (ins->rd)%4!=0)
        {
            xout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }


        int regFs1_1 = raex_latch->l_regFs1_1;
        int regFs1_2 = raex_latch->l_regFs1_2;
        int regFs1_3 = raex_latch->l_regFs1_3;
        int regFs1_4 = raex_latch->l_regFs1_4;

        long int FS1_msb = ((unsigned long int)regFs1_1<<32) | regFs1_2;
        long int FS1_lsb = ((unsigned long int)regFs1_3<<32) | regFs1_4;
        

        int regFs2_1 = raex_latch->l_regFs2_1;
        int regFs2_2 = raex_latch->l_regFs2_2;
        int regFs2_3 = raex_latch->l_regFs2_3;
        int regFs2_4 = raex_latch->l_regFs2_4;

        long int FS2_msb = ((unsigned long int)regFs2_1<<32) | regFs1_2;
        long int FS2_lsb = ((unsigned long int)regFs2_3<<32) | regFs1_4;
        
        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);
        long double num2 = llint_ldouble(FS2_msb, FS2_lsb);
        long double result;

        if(ins->opf==67){
             result = num1 + num2;
        }
        
        if(ins->opf==71){
            long double result = num1 - num2;
        }

        if(ins->opf==75){
             result = num1 * num2;
        }

        if(ins->opf==79){
            if(num2 == 0){
                xout << "DIVIDE BY ZERO" <<endl;
                return fp_exception;
            }
            result = num1 / num2;
        }
        
        long double result2 = result;


        exme_latch->rw_pack.F_nWords = 4;
        exme_latch->rw_pack.F_RD1 = ins->rd;
        exme_latch->rw_pack.F_RD1_val = *((int *)&result2);
        exme_latch->rw_pack.F_RD2 = ins->rd+1;
        exme_latch->rw_pack.F_RD2_val = *(((int *)&result2)+1);
        exme_latch->rw_pack.F_RD3 = ins->rd+2;
        exme_latch->rw_pack.F_RD3_val = *(((int *)&result2)+2);
        exme_latch->rw_pack.F_RD4 = ins->rd+3;
        exme_latch->rw_pack.F_RD4_val = *(((int *)&result2)+3);
        exme_latch->rw_pack.isFltReg = true;

    }

    if((ins->opf==73 || ins->opf==105) && ins->op3==52){
        //FMULs, FsMULd

        int regFs1 = raex_latch->l_regFs1_1;
        regFs2 = raex_latch->l_regFs2_1;

        float num1 = int_float(regFs1);
        float num2 = int_float(regFs2);
        
        if(ins->opf==73){
            float result = num1 * num2;
            exme_latch->rw_pack.F_nWords = 1;
            exme_latch->rw_pack.F_RD1 = ins->rd;
            exme_latch->rw_pack.F_RD1_val = *(int *)&result;
            exme_latch->rw_pack.isFltReg = true;

        }
        
        else
        {
            if(is_register_mis_aligned(ins->rd))
            {
                xout << ("Source is an odd-even register pair float double instruction") << endl;
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;

                return fp_exception;
            }
        
            double result = (double)num1 * (double)num2;

            exme_latch->rw_pack.F_nWords = 2;
            exme_latch->rw_pack.F_RD1 = ins->rd;
            exme_latch->rw_pack.F_RD1_val = *((unsigned long int *)&result)>>32;
            exme_latch->rw_pack.F_RD2 = ins->rd+1;
            exme_latch->rw_pack.F_RD2_val = *((unsigned long int *)&result);
            exme_latch->rw_pack.isFltReg = true;
       
        }
        
    }
    
    if((ins->opf==74 || ins->opf==110) && ins->op3==52){
        //FMULd, FdMULq
        if(is_register_mis_aligned(ins->rs1)||is_register_mis_aligned(ins->rs2)||is_register_mis_aligned(ins->rd))
        {
            xout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }
        

        int regFs1_msb = raex_latch->l_regFs1_1;
        int regFs1_lsb = raex_latch->l_regFs1_2;

        long int FS1 = (((long int)regFs1_msb)<<32) | regFs1_lsb;


        int regFs2_msb = raex_latch->l_regFs2_1;
        int regFs2_lsb = raex_latch->l_regFs2_2;

        long int Fs2 = (((long int)regFs2_msb)<<32) | regFs2_lsb;

        double num1 = lint_double(FS1);
        double num2 = lint_double(Fs2);
        if(ins->opf==74){
            double result = num1 * num2;

            exme_latch->rw_pack.F_nWords = 2;
            exme_latch->rw_pack.F_RD1 = ins->rd;
            exme_latch->rw_pack.F_RD1_val = *((unsigned long int *)&result)>>32;
            exme_latch->rw_pack.F_RD2 = ins->rd+1;
            exme_latch->rw_pack.F_RD2_val = *((unsigned long int *)&result);
            exme_latch->rw_pack.isFltReg = true;
       
        }
        else
        {
            if((ins->rd)%4!=0){
                xout << ("Source is an odd-even register pair float double instruction") << endl;
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;

                return fp_exception;
            }

            long double result = num1 * num2;
            long double result2 = result;

            exme_latch->rw_pack.F_nWords = 4;
            exme_latch->rw_pack.F_RD1 = ins->rd;
            exme_latch->rw_pack.F_RD1_val = *((int *)&result2);
            exme_latch->rw_pack.F_RD2 = ins->rd+1;
            exme_latch->rw_pack.F_RD2_val = *(((int *)&result2)+1);
            exme_latch->rw_pack.F_RD3 = ins->rd+2;
            exme_latch->rw_pack.F_RD3_val = *(((int *)&result2)+2);
            exme_latch->rw_pack.F_RD4 = ins->rd+3;
            exme_latch->rw_pack.F_RD4_val = *(((int *)&result2)+3);
            exme_latch->rw_pack.isFltReg = true;
        }
    }

    if((ins->opf==81 || ins->opf==85) && ins->op3==53){
        //FCMPs, FCMPEs

        int regFs1 = raex_latch->l_regFs1_1;
        regFs2 =raex_latch->l_regFs2_1;

        float num1 = int_float(regFs1);
        float num2 = int_float(regFs2);
        if(num1==num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        if(num1<num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        if(num1>num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        
        if(is_nan_int(regFs1)!=0 || is_nan_int(regFs2)!=0){
            //Nan!

            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(3, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            if(ins->opf==85){
                xout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
            else if(is_nan_int(regFs1)==1 || is_nan_int(regFs2)==1){
                xout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
        }
        if(is_infinity_int(regFs1)!=0 || is_infinity_int(regFs2)!=0){
            
            int num1 = is_infinity_int(regFs1);
            int num2 = is_infinity_int(regFs2);

            if(num1==num2){
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;
            }
            if(num1<num2){
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;
            }
            if(num1>num2){
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;
            }
        }
    }

    if((ins->opf==82 || ins->opf==86) && ins->op3==53){
        //FCMPd, FCMPEd
        if(is_register_mis_aligned(ins->rs1)||is_register_mis_aligned(ins->rs2))
        {
            xout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            return fp_exception;
        }
        

        int regFs1_msb = raex_latch->l_regFs1_1;
        int regFs1_lsb = raex_latch->l_regFs1_2;

        long int FS1 = ((long int)regFs1_msb<<32) | regFs1_lsb;

        int regFs2_msb = raex_latch->l_regFs2_1;
        int regFs2_lsb = raex_latch->l_regFs2_2;

        long int FS2 = ((long int)regFs2_msb<<32) | regFs2_lsb;

        double num1 = lint_double(FS1);
        double num2 = lint_double(FS2);

        if(num1==num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        if(num1<num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        if(num1>num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        
        if(is_nan_lint(FS1)!=0 || is_nan_lint(FS2)!=0){
            //Nan!

            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(3, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            if(ins->opf==86){
                xout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
            else if(is_nan_lint(FS1)==1 || is_nan_lint(FS2)==1){
                xout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
        }
        
        if(is_infinity_lint(FS1)!=0 || is_infinity_lint(FS2)!=0){
            
            int num1 = is_infinity_lint(FS1);
            int num2 = is_infinity_lint(FS2);

            if(num1==num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
            }
            if(num1<num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
            }
            if(num1>num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
            }
        }
        
    }

    if((ins->opf==83 || ins->opf==87) && ins->op3==53){
        //FCMPq, FCMPEq,
        
        if((ins->rs1)%4!=0 || (ins->rs2)%4!=0)
        {
            xout << ("Source is an odd-even register pair float quad instruction") << endl;
                // sregister->SRegisters.fsr->setFtt(INVALID_FP_REGISTER);
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        
            return fp_exception;
        }


        int regFs1_1 = raex_latch->l_regFs1_1;
        int regFs1_2 = raex_latch->l_regFs1_2;
        int regFs1_3 = raex_latch->l_regFs1_3;
        int regFs1_4 = raex_latch->l_regFs1_4;

        long int FS1_msb = ((long int)regFs1_1<<32) | regFs1_2;
        long int FS1_lsb = ((long int)regFs1_3<<32) | regFs1_4;
        
  
        int regFs2_1 = raex_latch->l_regFs2_1;
        int regFs2_2 = raex_latch->l_regFs2_2;
        int regFs2_3 = raex_latch->l_regFs2_3;
        int regFs2_4 = raex_latch->l_regFs2_4;

        long int FS2_msb = ((long int)regFs2_1<<32) | regFs1_2;
        long int FS2_lsb = ((long int)regFs2_3<<32) | regFs1_4;
        
        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);
        long double num2 = llint_ldouble(FS2_msb, FS2_lsb);
        if(num1==num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        if(num1<num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        if(num1>num2){
            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;
        }
        
        if(is_nan_llint(FS1_msb, FS1_lsb)!=0 || is_nan_llint(FS2_msb,FS2_lsb)!=0){
            //Nan!

            int t_fsr = raex_latch->l_FSR;
            t_fsr = modifyBits(3, t_fsr, FSR_fcc_l, FSR_fcc_r); 
            exme_latch->rw_pack.FSR = t_fsr;
            exme_latch->rw_pack.isFSR = true;

            if(ins->opf==87){
                xout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
            else if(is_nan_llint(FS1_msb, FS1_lsb)==1 || is_nan_llint(FS2_msb,FS2_lsb)==1){
                xout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
        }
        
        if(is_infinity_llint(FS1_msb, FS1_lsb)!=0 || is_infinity_llint(FS2_msb,FS2_lsb)!=0){
            
            int num1 = is_infinity_llint(FS1_msb, FS1_lsb);
            int num2 = is_infinity_llint(FS2_msb,FS2_lsb);

            if(num1==num2){
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;
            }
            if(num1<num2){
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;
            }
            if(num1>num2){
                int t_fsr = raex_latch->l_FSR;
                t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r); 
                exme_latch->rw_pack.FSR = t_fsr;
                exme_latch->rw_pack.isFSR = true;
            }
        }
        
        
    }

    if(ins->opf==41 || ins->opf==42 || ins->opf==43){
        //FSQRTs, FSQRTd, FSQRTq
        
        // if(is_register_mis_aligned(ins->rs1)||is_register_mis_aligned(ins->rs2)||is_register_mis_aligned(ins->rd))
        // {
        //     xout << ("Source is an odd-even register pair float double instruction") << endl;
        //     sregister->SRegisters.fsr->setFtt(INVALID_FP_REGISTER);
        //     return fp_exception;
        // }
    }

    return RET_SUCCESS;


}



/*
 * Updates Integer Condition Code (ICC) bits based on the result of addition. op==0 for add, op==1 for subtract
 */
void EX::updateICCAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int op)
{
    unsigned int  t_psr = raex_latch->l_PSR;

	int signBit_regRD = getBit(regRD, SIGN_BIT);

	// Set ICC_NEGATIVE (n) bit
    t_psr = modifyBits(signBit_regRD, t_psr, 23, 23);

	// Set ICC_ZERO (z) bit
    if(regRD == 0){ 

        t_psr = modifyBits(1, t_psr, 22, 22);


        } else {
        t_psr = modifyBits(0, t_psr, 22, 22);
        }

	// Set ICC_OVERFLOW (v) bit: Important for SIGNED arithmetic
    if(op==0){
        //add
        if(addOverflow(regRS1, reg_or_imm, regRD)){ 
        t_psr = modifyBits(1, t_psr, 21, 21);
        }
        else {
        t_psr = modifyBits(0, t_psr, 21, 21);
        }
    }
    else{
        //subtract
        if(subtractOverflow(regRS1, reg_or_imm, regRD)){
        t_psr = modifyBits(1, t_psr, 21, 21);
        }
        else {
        t_psr = modifyBits(0, t_psr, 21, 21);
        }

    }
    
	// Set ICC_CARRY (c) bit: Important for UNSIGNED arithmetic
    if(op==0){
        if(addCarry(regRS1, reg_or_imm, regRD)){  
        t_psr = modifyBits(1, t_psr, 20, 20);
        }
        else {
        t_psr = modifyBits(0, t_psr, 20, 20);
        }

    }
    else{
        if(subtractCarry(regRS1, reg_or_imm, regRD)){
        t_psr = modifyBits(1, t_psr, 20, 20);
        }
        else {
        t_psr = modifyBits(0, t_psr, 20, 20);
        }

    }


    exme_latch->rw_pack.PSR = t_psr;
    exme_latch->rw_pack.isPSR = true;
}


/*
 * Updates Integer Condition Code (ICC) bits based on the result of tagged addition and subtraction.
 * If isTVOpcode = 1, the opcodes will be treated as TADDCCTV and TSUBCCTV.
 * If isTVOpcode = 0, the opcodes will be treated as TADDCC and TSUBCC.
 * Returns 1, if tagged overflow has occurred.
 */
//op=0 for add
int EX::taggedAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int isTVOpcode, int op)
{
	int signBit_regRS1, signBit_reg_or_imm, signBit_regRD, istag_overflow, isTaggedOverflow;
	

	signBit_regRS1 = getBit(regRS1, SIGN_BIT);
	signBit_reg_or_imm = getBit(reg_or_imm, SIGN_BIT);
	signBit_regRD = getBit(regRD, SIGN_BIT);
    
    if(getBit(regRS1, 0)==1 || getBit(regRS1, 1)==1 || getBit(reg_or_imm, 0)==1 || getBit(reg_or_imm, 1)==1) 
        istag_overflow = 1;
    else
        istag_overflow = 0;
    
    // Set ICC_OVERFLOW (v) bit: Important for TAGGED arithmetic
    if(op == 0){
        
        if((addOverflow(regRS1, reg_or_imm, regRD)==1 || istag_overflow==1))
            isTaggedOverflow = 1;
        else
            isTaggedOverflow = 0;
    }
    else{
        
        if(subtractOverflow(regRS1, reg_or_imm, regRD)==1 || istag_overflow==1)
            isTaggedOverflow = 1;
        else
            isTaggedOverflow = 0;
    }
        
    if(isTaggedOverflow==1 && isTVOpcode==1)
        return RET_TRAP;
    else
    {

        updateICCAddSubtract(regRS1,reg_or_imm, regRD, op);
        int t_psr=raex_latch->l_PSR;
        if(isTaggedOverflow==1) {
            t_psr = modifyBits(1, t_psr, 21, 21);
            }
        else {
            t_psr = modifyBits(0, t_psr, 21, 21);

        }
        exme_latch->rw_pack.PSR = t_psr;
        exme_latch->rw_pack.isPSR = true;        

    }
    return isTaggedOverflow;     
}

/*
 * Updates Integer Condition Code (ICC) bits based on the result of multiplication and logical operations.
 */
void EX::updateICCMulLogical(regType regRD)
{
	int signBit_regRD = getBit(regRD, SIGN_BIT);
    
    int t_psr=raex_latch->l_PSR;

	// Set ICC_NEGATIVE (n) bit
    t_psr = modifyBits(signBit_regRD, t_psr, 23, 23);

	// Set ICC_ZERO (z) bit
    if(regRD == 0) {
        t_psr = modifyBits(1, t_psr, 22, 22);
    }
    else {
        t_psr = modifyBits(0, t_psr, 22, 22);
        }

	// Set ICC_OVERFLOW (v) bit: Important for SIGNED arithmetic
    t_psr = modifyBits(0, t_psr, 21, 21);

	// Set ICC_CARRY (c) bit: Important for UNSIGNED arithmetic
    t_psr = modifyBits(0, t_psr, 20, 20);

    exme_latch->rw_pack.PSR = t_psr;
    exme_latch->rw_pack.isPSR = true;
}

/*
 * Updates Integer Condition Code (ICC) bits based on the result of division.
 */
void EX::updateICCDiv(regType regRD, int isOverflow)
{
    int t_psr = raex_latch->l_PSR;
	int signBit_regRD = getBit(regRD, SIGN_BIT);
	// Set ICC_NEGATIVE (n) bit
    t_psr = modifyBits(signBit_regRD, t_psr, 23, 23);

	
    // Set ICC_ZERO (z) bit
    if(regRD == 0) {
    t_psr = modifyBits(1, t_psr, 22, 22);

    }
    else {
    t_psr = modifyBits(0, t_psr, 22, 22);
        }
	
    // Set ICC_OVERFLOW (v) bit: Important for SIGNED arithmetic
    if(isOverflow){
    t_psr = modifyBits(1, t_psr, 21, 21);

    }
    else {
    t_psr = modifyBits(0, t_psr, 21, 21);
        }
	
    // Set ICC_CARRY (c) bit: Important for UNSIGNED arithmetic
    t_psr = modifyBits(0, t_psr, 20, 20);

    exme_latch->rw_pack.PSR = t_psr;
    exme_latch->rw_pack.isPSR = true;

}

void EX::perform() 
{
    if(raex_latch->isEX_Enabled == false){xout<<"EX STALL\n";return;}
    else{xout<<"EX start\n";}


    instruction* ins  = &(raex_latch->l_Ins);


    
    int returnValue = RET_SUCCESS;

    if(ins->op==1){
        //TODO call
        CallInstruction(ins);//Checked correct

    }
    else if(ins->op==0){
        //op2
        //0 - UNIMP
        //1 - unnimplemented
        //3 - unnimplemented
        //5 - unnimplemented
        //7 - CBcc
        if(ins->op2 == 4){
            // SethiNop(ins, sregister, memory); //checked correct
            SethiNop(ins); //checked correct

            exme_latch->rw_pack.PC = raex_latch->l_nPC ;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.isCounter = true;
        }
        else{
            if(ins->op2==2)
                BranchIntegerInstructions(ins); //EVERYTHING HERE IS DONE EXCEPT THAT BICC SHOULD NOT BE PLACED IN DELAY SLOT OF CONDITIONAL BRANCH
            else if(ins->op2 == 6){
                returnValue = BranchFloatInstructions(ins);
            } 
            
        }
    }
    else{
        if(ins->op==3){ //memory instructions
            if(ins->op3==0 || ins->op3==1 || ins->op3==2 ||ins->op3==3 ||ins->op3==9 ||ins->op3==10 )
                returnValue = LoadIntegerInstructions(ins); //PRIVILEDGED LEFT
            else if(ins->op3==32 || ins->op3==33 || ins->op3==35 )
                returnValue = LoadFloatingPointInstructions(ins); //LDFSR INS LEFT,<< >> ISSUE
            else if(ins->op3==4 || ins->op3==5 || ins->op3==6 || ins->op3==7 )
                returnValue=StoreIntegerInstructions(ins); // PSR, PRIVILEDGED INSTRUCTIONS LEFT
            else if(ins->op3==36 || ins->op3==39 || ins->op3==37 || ins->op3==38)
                returnValue=StoreFloatInstructions(ins); //STDFQ Left
            else if(ins->op3 == 13 || ins->op3 == 29){
                returnValue=AtomicLoadStoreUnsignedByte(ins);//LDSTUB S bit in PSR not checked, LDSTUBA is left
            }
            else{
                returnValue=SWAP(ins); //PSR, PRIVILEDGED LEFT
            }
            

            exme_latch->rw_pack.PC = raex_latch->l_nPC ;
            exme_latch->rw_pack.nPC = raex_latch->l_nPC+4;
            exme_latch->rw_pack.isCounter = true;

        }
        //arithmetic operations
        else{ //IN THIS CASE OP==2
            
                if(ins->op3==56) 
                    returnValue=JumpAndLink(ins); //SEEMS OKAY

                else if (ins->op3==57)
                    returnValue = RETT(ins); //NOT COMPLETE YET THE ERROR_MODE NOT IMPLEMENTED
                
                else{
                    
                        if(ins->op3==1 || ins->op3==17 || ins->op3==5 ||ins->op3==21 ||ins->op3==2 ||ins->op3==18
                        ||ins->op3==6 ||ins->op3==22 ||ins->op3==3 ||ins->op3==19 ||ins->op3==7||ins->op3==23)
                            LogicalInstructions(ins); //CHECKED, CORRECT PERFECT
                        
                        else if(ins->op3==58)
                            returnValue=TrapOnICC(ins); //SOME DISABLES TRAP DECREMENT CWP etc left.
                        
                        else if(ins->op3==40 || ins->op3==41 || ins->op3==42 || ins->op3==43)
                            ReadStateRegisterInstructions(ins);//ONLY RDY IS IMPLEMENTED.
                        
                        else if(ins->op3==48||ins->op3==49||ins->op3==50)
                            WriteStateRegisterInstructions(ins);//ONLY WRY IS IMPLEMENTED.
                        
                        else if(ins->op3==52 || ins->op3==53){
                            FpopInstructions(ins);
                        }

                        else if(ins->op3 == 37 || ins->op3 == 38 || ins->op3 == 39)
                            ShiftInstructions(ins);
                        
                        else if(ins->op3 == 0 || ins->op3 == 16 || ins->op3 == 8 || ins->op3 == 24)
                            AddInstructions(ins); //CHECKED CORRECT PERFECT
                        
                        else if(ins->op3 == 32 || ins->op3 == 34)
                            returnValue = TaggedAddInstructions(ins);//CHECKED seems CORRECT
                        
                        else if(ins->op3 == 4 || ins->op3 == 20 || ins->op3 == 12 || ins->op3 == 28) 
                            SubtractInstructions(ins); //Checked Correct Perfect
                        
                        else if(ins->op3 == 33 || ins->op3 == 35)
                            returnValue = TaggedSubtractInstructions(ins);///Checked seems correct
                        
                        else if(ins->op3 == 36)
                            MultiplyStepInstruction(ins);//Checked, correct
                        
                        else if(ins->op3 == 10 || ins->op3 == 11 || ins->op3 == 26 || ins->op3 == 27)
                            MultiplyInstructions(ins);//Checked, correct
                        
                        else if(ins->op3 == 14 || ins->op3 == 30 || ins->op3 == 15 || ins->op3 == 31)
                            returnValue = DivideInstructions(ins); //SDIV AND SDIVcc done but doubt in their overflow!!
                        
                        else if(ins->op3 == 60 || ins->op3 == 61)  
                            returnValue= SaveAndRestoreInstructions(ins); //checked PERFECT!!

                        if(returnValue!=RET_SUCCESS){
                            exme_latch->retVal = returnValue;
                            bzero(raex_latch,sizeof(RA_EX_latch));
                            raex_latch->isEX_Enabled = false;
                            exme_latch->isME_Enabled = true;
                            return;
                        }

                        exme_latch->rw_pack.PC = raex_latch->l_nPC ;
                        exme_latch->rw_pack.nPC = raex_latch->l_nPC+4;
                        exme_latch->rw_pack.isCounter = true;               
                
                    }
                    
            
            }
    }
    
    exme_latch->Ins = *ins;

    exme_latch->xc_pack = raex_latch->xc_pack;

    exme_latch->retVal = returnValue;

    bzero(raex_latch,sizeof(RA_EX_latch));
    raex_latch->isEX_Enabled = false;
    exme_latch->isME_Enabled = true;
    return;
}


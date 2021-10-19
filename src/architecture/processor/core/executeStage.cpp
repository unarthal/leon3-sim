#include "architecture/processor/core/core.h"
#include <iostream>
#include "architecture/processor/core/registerAccessStage.h"
#include "architecture/system.h"
#include "generic/utility.h"
#include "architecture/processor/core/registers/registerfile.h"
#include "architecture/processor/core/executeStage.h"
#include <climits>
#include "architecture/memory/memory.h"
#include "architecture/constants_typedefs.h"
#include <cfenv>
#include "architecture/processor/core/controllockUnit.h"

using namespace std;

executeStage::executeStage(core* x_containingCore) : element()
{
	m_containingCore = x_containingCore;
	m_registerAccessStage_executeStage_interface = 0;
	m_executeStage_memoryStage_interface = 0;
	m_memoryStage = m_containingCore->getMemoryStage();
	m_multicycleExecutionInProgress = false;
	m_hasTrapOccurred = false;

	m_eventQueue = new std::priority_queue<executioncompleteevent*, std::vector<executioncompleteevent*>, eventcompare>();
	multiplyLatency = 2;//TODO read from config file
	divideLatency = 35;//TODO read from config file

	pmc_branch = 0;
	pmc_takenBranch = 0;
}

executeStage::~executeStage()
{

}

extern int verbose;

inline int executeStage::addOverflow(regType regRS1, regType reg_or_imm, regType regRD)
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

inline int executeStage::subtractOverflow(regType regRS1, regType reg_or_imm, regType regRD)
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

inline int executeStage::addCarry(regType regRS1, regType reg_or_imm, regType regRD)
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

inline int executeStage::subtractCarry(regType regRS1, regType reg_or_imm, regType regRD)
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

int executeStage::LoadIntegerInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    addrType memoryAddress =
        ((unsigned int)regRS1) + imm_or_reg;

    m_msg->m_addr = memoryAddress;
    m_msg->isLoad = true;
    m_msg->isInt = true;
    m_msg->isMemIns = true;

    return RET_SUCCESS;
}

int executeStage::LoadFloatingPointInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    if(e_msg->l_PSR_EF == 0)
    {
        cout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }

    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    addrType memoryAddress =
        ((unsigned int)regRS1) + imm_or_reg;

    m_msg->m_addr = memoryAddress;
    m_msg->isLoad = true;
    m_msg->isFloat = true;
    m_msg->isMemIns = true;

    return RET_SUCCESS;
}

int executeStage::StoreIntegerInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType regRD = e_msg->l_regRD;

    regType imm_or_reg, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    addrType memoryAddress =
        (unsigned int)regRS1 + imm_or_reg;

    m_msg->m_addr = memoryAddress;
    m_msg->isStore = true;
    m_msg->isInt = true;
    m_msg->regRD = e_msg->l_regRD;
    m_msg->regNextRD = e_msg->l_regNextRD;
    m_msg->isMemIns = true;

    return RET_SUCCESS;
}

int executeStage::StoreFloatInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    if(e_msg->l_PSR_EF==0)
    {
        cout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }

 	regType regRD = e_msg->l_regRD;
    regType imm_or_reg, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    addrType memoryAddress =
        (unsigned int) regRS1 + imm_or_reg;

    m_msg->m_addr = memoryAddress;
    m_msg->isStore = true;
    m_msg->isFloat = true;
    m_msg->regRD = e_msg->l_regRD;
    m_msg->regNextRD = e_msg->l_regNextRD;
    m_msg->FSR = e_msg->l_FSR;
    m_msg->isMemIns = true;

    return RET_SUCCESS;
}

int executeStage::AtomicLoadStoreUnsignedByte(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{

    regType imm_or_reg, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    addrType memoryAddress =
        (unsigned int)regRS1 + imm_or_reg;

    m_msg->m_addr = memoryAddress;
    m_msg->isAtomic = true;
    m_msg->isMemIns = true;


    return RET_SUCCESS;
}

int executeStage::SWAP(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{

    regType regRD;
    regType imm_or_reg, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    addrType memoryAddress =
        (unsigned int)regRS1 + imm_or_reg;

    m_msg->m_addr = memoryAddress;
    m_msg->isSwap = true;
    m_msg->regRD = e_msg->l_regRD;
    m_msg->isMemIns = true;

    return RET_SUCCESS;

}

void executeStage::SethiNop(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    if((e_msg->getEInst()->rd == -1) && (e_msg->getEInst()->imm22 == -1))
    {
        //nop: rd and imm22 changed to -1 in decode stage
    }
    else
    {
        //sethi
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_isIntReg = true;
        m_msg->rw_RD_val =
            ((e_msg->getEInst()->imm22)<<10);
    }
}

void executeStage::LogicalInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    if(e_msg->getEInst()->op3 == 1)
    {
        //AND opcode:000001
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            (regRS1 & imm_or_reg);
    }
    else if(e_msg->getEInst()->op3 == 17)
    {
        //ANDcc opcode:010001
        result = regRS1 & imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        updateICCMulLogical(result, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3 == 5)
    {
        //ANDN opcode:000101
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            regRS1 & (~imm_or_reg);
    }
    else if(e_msg->getEInst()->op3 == 21)
    {
        //ANDNcc opcode:010101
        result = regRS1 & (~imm_or_reg);
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        updateICCMulLogical(result, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3 == 2)
    {
        //OR opcode:000010
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            regRS1 | imm_or_reg;
    }
    else if(e_msg->getEInst()->op3 == 18)
    {
        //ORcc opcode:010010
        result = regRS1 | imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        updateICCMulLogical(result, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3 == 6)
    {
        //ORN opcode:000110
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            regRS1 | (~imm_or_reg);
    }
    else if(e_msg->getEInst()->op3 == 22)
    {
        //ORNcc opcode:010110
        result = regRS1 | (~imm_or_reg);
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        updateICCMulLogical(result, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3 == 3)
    {
        //XOR opcode:000011
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            regRS1 ^ imm_or_reg;
    }
    else if(e_msg->getEInst()->op3 == 19)
    {
        //XORcc opcode:010011
        result = regRS1 ^ imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        updateICCMulLogical(result, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3 == 7)
    {
        //XNOR opcode:000111
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = ~(regRS1 ^ (imm_or_reg));
    }
    else if(e_msg->getEInst()->op3 == 23)
    {
        //XNORcc opcode:010111
        result = ~(regRS1 ^ (imm_or_reg));
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        updateICCMulLogical(result, e_msg, m_msg);
    }
    m_msg->rw_isIntReg = true;
}

void executeStage::ShiftInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    imm_or_reg = imm_or_reg & 0x0000001F;

    if(e_msg->getEInst()->op3 == 37)
    {
        //SLL opcode:100101
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            regRS1 << imm_or_reg;
    }
    else if(e_msg->getEInst()->op3 == 38)
    {
        //SRL opcode:100110
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            ((unsigned int)regRS1) >> imm_or_reg;
    }
    else if(e_msg->getEInst()->op3 == 39)
    {
        //SRA opcode:100111
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val =
            ((int)regRS1 >> imm_or_reg);
    }
    m_msg->rw_isIntReg = true;
}

void executeStage::AddInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    if(e_msg->getEInst()->op3==0)
    {
        //ADD opcode:000000
        result=regRS1+imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
    }
    else if(e_msg->getEInst()->op3==16)
    {
        // ADDcc opcode:010000
        result=regRS1+imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        updateICCAddSubtract(regRS1, imm_or_reg, result,0, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3==8)
    {
        // ADDX opcode:001000
        int carry = e_msg->l_PSR_icc_C;

        result=regRS1 + imm_or_reg + carry;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
    }
    else if(e_msg->getEInst()->op3==24)
    {
        // ADDXcc opcode:011000
        int carry = e_msg->l_PSR_icc_C;

        result=regRS1 + imm_or_reg + carry;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;

        updateICCAddSubtract(regRS1, imm_or_reg, result,0, e_msg, m_msg);
    }
    m_msg->rw_isIntReg = true;
}

int executeStage::TaggedAddInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    if(e_msg->getEInst()->op3==32)
    {
        // TADDcc opcode:100000
        result=regRS1 + imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        taggedAddSubtract(regRS1, imm_or_reg, result, 0, 0, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3==34)
    {
        // TADDccTV opcode:100010
        result=regRS1 + imm_or_reg;
        if(taggedAddSubtract(regRS1, imm_or_reg, result, 1, 0, e_msg, m_msg)==RET_TRAP)
        {
            cout << "Tag overflow has occurred\n";
            return tag_overflow;
        }
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;

    }
    m_msg->rw_isIntReg = true;
    return RET_SUCCESS;
}

void executeStage::SubtractInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    if(e_msg->getEInst()->op3==4)
    {
        //SUB opcode:000100
        result=regRS1 - imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
    }
    else if(e_msg->getEInst()->op3==20)
    {
        // SUBcc opcode:010100
        result=regRS1 - imm_or_reg;

        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;

        updateICCAddSubtract(regRS1, imm_or_reg, result,1, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3==12)
    {
        // SUBX opcode:001100
        int carry = e_msg->l_PSR_icc_C;

        result=regRS1 - imm_or_reg - carry;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
    }
    else if(e_msg->getEInst()->op3==28)
    {
        // SUBXcc opcode:011100
        int carry = e_msg->l_PSR_icc_C;

        result=regRS1 - imm_or_reg - carry;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;

        updateICCAddSubtract(regRS1, imm_or_reg, result,1, e_msg, m_msg);
    }
    m_msg->rw_isIntReg = true;
}

int executeStage::TaggedSubtractInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    if(e_msg->getEInst()->op3==33)
    {
        // TSUBcc opcode:100001
        result=regRS1 - imm_or_reg;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        taggedAddSubtract(regRS1, imm_or_reg, result, 0, 1, e_msg, m_msg);
    }
    else if(e_msg->getEInst()->op3==35)
    {
        // TSUBccTV opcode:100011
        result=regRS1 - imm_or_reg;
        if(taggedAddSubtract(regRS1, imm_or_reg, result, 1, 1, e_msg, m_msg)==RET_TRAP)
        {
            cout << ("Tag overflow has occurred") << endl;
            return tag_overflow;
        }
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
    }
    m_msg->rw_isIntReg = true;
    return RET_SUCCESS;

}

void executeStage::MultiplyStepInstruction(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{//SOME CONFUSION IN STEP 3
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    if(e_msg->getEInst()->op3 == 36){
        //MULScc opcode:100100

        regType y = e_msg->l_Y;

        int lsb_regRS1 = getBit(regRS1,0);

        int sign_bit = (e_msg->l_PSR_icc_N) ^ (e_msg->l_PSR_icc_V);

        sign_bit = sign_bit & 0x00000001;

        regRS1= regRS1>>1;
        regRS1 = modifyBit(regRS1, 31, sign_bit);

        if(getBit(y,0)==1){
            m_msg->rw_RD = e_msg->getEInst()->rd;
            m_msg->rw_RD_val = regRS1 + imm_or_reg;
            updateICCAddSubtract(regRS1, imm_or_reg, regRS1 + imm_or_reg , 0, e_msg, m_msg);
        }
        else{
            m_msg->rw_RD = e_msg->getEInst()->rd;
            m_msg->rw_RD_val = regRS1;
            updateICCAddSubtract(regRS1, 0, regRS1, 0, e_msg, m_msg);
        }
        m_msg->rw_isIntReg = true;


        y = y>>1;
        y = modifyBit(y, 31, lsb_regRS1);
        m_msg->rw_Y = y;
        m_msg->rw_isY = true;
    }
}

void executeStage::MultiplyInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{

    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    if(e_msg->getEInst()->op3 == 10){
        //UMUL opcode:001010
        unsigned long extended_regRD;
        regType regY =0;
        extended_regRD = (unsigned long)regRS1 * (unsigned long)imm_or_reg;

        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        m_msg->rw_Y = regY;
        m_msg->rw_isY = true;

        result = extended_regRD & 0x00000000FFFFFFFF;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        m_msg->rw_isIntReg = true;


    }

    else if(e_msg->getEInst()->op3==11){
        //SMUL opcode:001011
        signed long extended_regRD;
        regType regY = 0;
        extended_regRD = (signed long)regRS1 * (signed long)imm_or_reg;

        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        m_msg->rw_Y = regY;
        m_msg->rw_isY = true;

        result = extended_regRD & 0x00000000FFFFFFFF;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        m_msg->rw_isIntReg = true;
    }

    else if(e_msg->getEInst()->op3 == 26){
        //UMULcc opcode:011010
        unsigned long extended_regRD;
        regType regY = 0;
        extended_regRD = (unsigned long)regRS1 * (unsigned long)imm_or_reg;

        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        m_msg->rw_Y = regY;
        m_msg->rw_isY = true;

        result = extended_regRD & 0x00000000FFFFFFFF;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        m_msg->rw_isIntReg = true;

        updateICCMulLogical(result, e_msg, m_msg);

    }

    else if(e_msg->getEInst()->op3==27){
        //SMULcc opcode:011011
        signed long long extended_regRD;
        regType regY = 0;
        extended_regRD = (signed long)regRS1 * (signed long)imm_or_reg;

        regY = (extended_regRD & 0xFFFFFFFF00000000) >> 32;
        m_msg->rw_Y = regY;
        m_msg->rw_isY = true;

        result = extended_regRD & 0x00000000FFFFFFFF;
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        m_msg->rw_isIntReg = true;

        updateICCMulLogical(result, e_msg, m_msg);
    }
}

int executeStage::DivideInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    if(imm_or_reg == 0)
    {
            cout << ("Attempt to divide by zero") << endl;
            return division_by_zero;
    }

    if(e_msg->getEInst()->op3 == 14){
        //UDIV opcode:001110
        unsigned long dividend;

        dividend = e_msg->l_Y;

        dividend = (dividend << 32) | regRS1;

        result = (int)(dividend /((unsigned int) imm_or_reg));

        unsigned long result_overflow = dividend /((unsigned int) imm_or_reg);
        if(result_overflow > UINT_MAX){
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = 0xFFFFFFFF;
        }
        else{
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        }
    m_msg->rw_isIntReg = true;
    }

    else if(e_msg->getEInst()->op3 == 30){
        //UDIVcc opcode:011110
        unsigned long dividend;


        dividend = e_msg->l_Y;

        dividend = (dividend << 32) | regRS1;
        result = dividend /(unsigned int) imm_or_reg;

        unsigned long result_overflow = dividend /((unsigned int) imm_or_reg);
        if(result_overflow > UINT_MAX){
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = 0xFFFFFFFF;
            updateICCDiv(0xFFFFFFFF, 1, e_msg, m_msg);
        }
        else{
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
            updateICCDiv(result, 0, e_msg, m_msg);

        }
    m_msg->rw_isIntReg = true;
    }

    else if(e_msg->getEInst()->op3 == 15){
        //SDIV opcode:001111
        signed long dividend;
        dividend = e_msg->l_Y;

        dividend = (dividend << 32) | regRS1;
        result = dividend / imm_or_reg;

        signed long result_overflow = dividend / imm_or_reg;
        if(result_overflow > INT_MAX){
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = INT_MAX;
        }
        else if(result_overflow < (signed int)INT_MIN){
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = INT_MIN;
        }
        else{
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
        }
    m_msg->rw_isIntReg = true;
    }

    else if(e_msg->getEInst()->op3 == 31){
        //SDIVcc opcode:011111
        signed long dividend;


        dividend = e_msg->l_Y;

        dividend = (dividend << 32) | regRS1;
        result = dividend / imm_or_reg;
        signed long result_overflow = dividend / imm_or_reg;

        if(result_overflow > INT_MAX){
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = INT_MAX;
            updateICCDiv(INT_MAX, 1, e_msg, m_msg);
        }
        else if(result_overflow < INT_MIN){
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = INT_MIN;
            updateICCDiv(INT_MIN, 1, e_msg, m_msg);
        }
        else{
        m_msg->rw_RD = e_msg->getEInst()->rd;
        m_msg->rw_RD_val = result;
            updateICCDiv(result, 0, e_msg, m_msg);
        }

    m_msg->rw_isIntReg = true;

    }

    return RET_SUCCESS;

}

int executeStage::SaveAndRestoreInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
	regType imm_or_reg, result, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    if(e_msg->getEInst()->op3==60){

        //save op:111100

        short nextCWP;

        nextCWP = (e_msg->l_CWP - 1) % e_msg->l_nWP;

        if(nextCWP<0){
            nextCWP = nextCWP + e_msg->l_nWP;

        }
        // Test for window overflow.

        if(getBit(e_msg->l_WIM, nextCWP)==1)
        {
        	if(verbose == 2)
        	{
        		cout << ("Register window overflow has occurred") <<endl;
        	}
            return window_overflow;
        }
        // If window_overflow does not take place.
        else
        {
            int t_psr = e_msg->l_PSR;
            t_psr = modifyBits(nextCWP, t_psr, PSR_CWP_l, PSR_CWP_r);
            m_msg->rw_PSR = t_psr;
            m_msg->rw_isPSR = true;

            m_msg->rw_RD = e_msg->getEInst()->rd;
            m_msg->rw_RD_val = ((unsigned int)regRS1) + imm_or_reg;
            m_msg->rw_isIntReg = true;

            return RET_SUCCESS;
        }
    }

    else if(e_msg->getEInst()->op3==61)
	{

        //restore
        short nextCWP;

        nextCWP = (e_msg->l_CWP + 1)%e_msg->l_nWP;

        // Test for window underflow.

        if(getBit(e_msg->l_WIM, nextCWP)==1)
        {
        	if(verbose == 2)
			{
				cout << ("Register window underflow has occurred") << endl;
			}
            return window_underflow;
        }
        // If window_underflow does not take place.
        else
        {

            int t_psr = e_msg->l_PSR;
            t_psr = modifyBits(nextCWP, t_psr, PSR_CWP_l, PSR_CWP_r);
            m_msg->rw_PSR = t_psr;
            m_msg->rw_isPSR = true;

            m_msg->rw_RD = e_msg->getEInst()->rd;
            m_msg->rw_RD_val = (unsigned int)regRS1 + imm_or_reg;
            m_msg->rw_isIntReg = true;

            return RET_SUCCESS;
        }
	}


}

void executeStage::BranchIntegerInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    addrType regPC=e_msg->l_PC;

	int condition=0;
	if(e_msg->getEInst()->cond==8)
		condition = 1; //BA cond:1000

	if(e_msg->getEInst()->cond==9) //BNE cond:1001
		condition = ~e_msg->l_PSR_icc_Z;

	if(e_msg->getEInst()->cond==1) //BE cond:0001
		condition = e_msg->l_PSR_icc_Z;

	if(e_msg->getEInst()->cond==10){ //BG cond:1010
		condition = ~(e_msg->l_PSR_icc_Z | (e_msg->l_PSR_icc_N ^ e_msg->l_PSR_icc_V));

	}
	if(e_msg->getEInst()->cond==2) //BLE cond:0010
		condition = e_msg->l_PSR_icc_Z | (e_msg->l_PSR_icc_N ^ e_msg->l_PSR_icc_V);

	if(e_msg->getEInst()->cond==11) //BGE cond:1011
		condition = ~(e_msg->l_PSR_icc_N ^ e_msg->l_PSR_icc_V);

	if(e_msg->getEInst()->cond==3) //BL cond:0011
		condition = e_msg->l_PSR_icc_N ^ e_msg->l_PSR_icc_V;

	if(e_msg->getEInst()->cond==12)  //BGU cond:1100
		condition = ~(e_msg->l_PSR_icc_C | e_msg->l_PSR_icc_Z);

	if(e_msg->getEInst()->cond==4) //BLEU cond:0100
		condition = e_msg->l_PSR_icc_C | e_msg->l_PSR_icc_Z;

	if(e_msg->getEInst()->cond==13) //BCC cond:1101
		condition = ~e_msg->l_PSR_icc_C;

	if(e_msg->getEInst()->cond==5) //BCS cond:0101
		condition = e_msg->l_PSR_icc_C;

	if(e_msg->getEInst()->cond==14) //BPOS cond:1110
		condition = ~e_msg->l_PSR_icc_N;

	if(e_msg->getEInst()->cond==6) //BNEG cond:0110
		condition = ~e_msg->l_PSR_icc_N;

	if(e_msg->getEInst()->cond==15) //BVC cond:1111
		condition = ~e_msg->l_PSR_icc_V;

	if(e_msg->getEInst()->cond==7) //BVS cond:0111
		condition = ~e_msg->l_PSR_icc_V;

	condition = getBit(condition, 0);

	if(condition==1)
	{
		// Branch taken
		m_msg->rw_PC = regPC+e_msg->getEInst()->disp22*4;
		m_msg->rw_nPC = regPC+e_msg->getEInst()->disp22*4 + 4;
		m_msg->rw_isControl = true;
		pmc_takenBranch++;
		if(verbose == 2)
		{
			cout << "taken branch\n";
		}
	}

	if(e_msg->getEInst()->cond==8/*BA*/ || e_msg->getEInst()->cond==0/*BN*/)
	{
		if(e_msg->getEInst()->a==1)
		{
			m_containingCore->getControlLockUnit()->performAnnulment(e_msg->getEPC()+4);
		}
	}
	else/*conditional branch*/
	{
		if(condition == 0)/*not taken branch*/
		{
			if(e_msg->getEInst()->a==1)
			{
				m_containingCore->getControlLockUnit()->performAnnulment(e_msg->getEPC()+4);
			}
		}
	}

    pmc_branch++;
}

int executeStage::BranchFloatInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{//TODO

    addrType regPC=e_msg->l_PC;

    if(e_msg->l_PSR_EF==0){
        cout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }

    if(e_msg->getEInst()->cond==8){
        //FBA instruction
        unsigned int next_pc = e_msg->l_PC+4*((int)e_msg->getEInst()->disp22);//UNSIGNED INT REMOVED FROM HERE
        if(e_msg->getEInst()->a==1){

            m_msg->rw_PC = next_pc;
            m_msg->rw_nPC = next_pc+4;
            m_msg->rw_isControl = true;
        }
        else{

            m_msg->rw_PC = e_msg->l_nPC;
            m_msg->rw_nPC = next_pc;
            m_msg->rw_isControl = true;
        }
    }

    else if(e_msg->getEInst()->cond==0){
        //FBN instruction
        if(e_msg->getEInst()->a==1){

            m_msg->rw_PC = e_msg->l_nPC+4;
            m_msg->rw_nPC = e_msg->l_nPC+8;
            m_msg->rw_isControl = true;
        }
        else{

            m_msg->rw_PC = e_msg->l_nPC;
            m_msg->rw_nPC = e_msg->l_nPC+4;
            m_msg->rw_isControl = true;
        }
    }

    else{
        int condition = e_msg->l_FSR_fcc;

        bool is_branch = false;
        if(e_msg->getEInst()->cond==7); //FBU cond:0111
            if(condition==3)
                is_branch=true;
        if(e_msg->getEInst()->cond==6) //FBG cond:0110
            if(condition==2)
                is_branch=true;
        if(e_msg->getEInst()->cond==5) //FBUG cond:0101
            if(condition==2 || condition==3)
                is_branch=true;
        if(e_msg->getEInst()->cond==4) //FBL cond:0100
            if(condition==1)
                is_branch=true;
        if(e_msg->getEInst()->cond==3) //FUBL cond:0011
            if(condition==1 || condition==3)
                is_branch=true;
        if(e_msg->getEInst()->cond==2) //FBLG cond:0010
            if(condition==1 || condition==2)
                is_branch=true;
        if(e_msg->getEInst()->cond==1)  //FBNE cond:0001
            if(condition!=0)
                is_branch=true;
        if(e_msg->getEInst()->cond==9) //FBE cond:1001
            if(condition==0)
                is_branch=true;
        if(e_msg->getEInst()->cond==10) //FBUE cond:1010
            if(condition==3 || condition==0)
                is_branch=true;

        if(e_msg->getEInst()->cond==11) //FBGE cond:1011
            if(condition==0 || condition==2)
                is_branch=true;

        if(e_msg->getEInst()->cond==12) //FBUGE cond:1100
            if(condition==3 || condition==2 || condition==0)
                is_branch=true;

        if(e_msg->getEInst()->cond==13) //FBLE cond:1101
            if(condition==1 || condition==0)
                is_branch=true;

        if(e_msg->getEInst()->cond==14) //FBULE cond:1110
            if(condition!=2)
                is_branch=true;

        if(e_msg->getEInst()->cond==15) //FBO cond:1111
            if(condition==0 || condition==1 || condition==2)
                is_branch=true;

        if(is_branch){

            m_msg->rw_PC = e_msg->l_nPC;
            m_msg->rw_nPC = regPC+e_msg->getEInst()->disp22*4;
            m_msg->rw_isControl = true;
            pmc_takenBranch++;
		}

        else{
            // Branch NOT taken
            if(e_msg->getEInst()->a==1){

            m_msg->rw_PC = e_msg->l_nPC+4;
            m_msg->rw_nPC = e_msg->l_nPC+8;
            m_msg->rw_isControl = true;
            }
            else{
                // Annul bit = 0

            m_msg->rw_PC = e_msg->l_nPC;
            m_msg->rw_nPC = e_msg->l_nPC+4;
            m_msg->rw_isControl = true;
            }
        }
    }

   pmc_branch++;

   return RET_SUCCESS;
}

void executeStage::CallInstruction(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{

    addrType regPC=e_msg->l_PC;


    m_msg->rw_RD = 15;
    m_msg->rw_RD_val = regPC;
    m_msg->rw_isIntReg = true;

    m_msg->rw_PC = regPC+(int)(e_msg->getEInst()->disp30 << 2);
    m_msg->rw_nPC = regPC+(int)(e_msg->getEInst()->disp30 << 2) + 4;
    m_msg->rw_isControl = true;

    pmc_takenBranch++;
    pmc_branch++;

}

int executeStage::JumpAndLink(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{

    regType imm_or_reg, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    addrType memoryAddress =
        (unsigned int)regRS1 + imm_or_reg;

    if(is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
    {
        cout << ("Destination memory address not word aligned") << endl;
        return mem_address_not_aligned;
    }

    m_msg->rw_RD = e_msg->getEInst()->rd;
    m_msg->rw_RD_val = e_msg->l_PC;
    m_msg->rw_isIntReg = true;

    m_msg->rw_PC = memoryAddress;
    m_msg->rw_nPC = memoryAddress+4;
    m_msg->rw_isControl = true;

    pmc_takenBranch++;
    pmc_branch++;
    //cout << "JMPL: "<< sregister->getnPC()<<endl;

    return RET_SUCCESS;

}

int executeStage::RETT(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    addrType targetAddr =
        (unsigned int)regRS1 + imm_or_reg;//UNSIGNED INT REMOVED FROM HERE

    short nextCWP;

    nextCWP = (e_msg->l_CWP + 1)%e_msg->l_nWP;

    // Traps:
    if(e_msg->l_PSR_ET==1 && e_msg->l_PSR_S==0){
        cout << "privileged_instruction trap"<<endl;
        return privileged_instruction;
    }
    if(e_msg->l_PSR_ET==1 && e_msg->l_PSR_S==1){
        cout << "illegal_instruction trap"<<endl;
        return illegal_instruction;
    }
    if(e_msg->l_PSR_ET==0 && (e_msg->l_PSR_S==0)){
        cout << "privileged_instruction trap"<<endl;
        return privileged_instruction;
    }
    if(e_msg->l_PSR_ET==0 && getBit(e_msg->l_WIM, nextCWP)==1){
        cout << "window_underflow trap"<<endl;
        return window_underflow;
    }
    if(e_msg->l_PSR_ET==0 && is_mem_address_not_aligned(targetAddr,WORD_ALIGN)==1){
        cout << "mem_address_not_aligned trap"<<endl;
        return mem_address_not_aligned;
    }

    // If TRAPS does not take place.
    else
    {

        m_msg->rw_PC = targetAddr;
        m_msg->rw_nPC = targetAddr+4;
        m_msg->rw_isControl = true;
        pmc_takenBranch++;
        pmc_branch++;


        int t_psr = e_msg->l_PSR;
        t_psr = modifyBits(e_msg->l_PSR_PS, t_psr, PSR_S_l, PSR_S_r);
        t_psr = modifyBits(1, t_psr, PSR_ET_l, PSR_ET_r);
        t_psr = modifyBits(nextCWP, t_psr, PSR_CWP_l, PSR_CWP_r);
        m_msg->rw_PSR = t_psr;
        m_msg->rw_isPSR = true;

        return RET_SUCCESS;
    }
}


int executeStage::TrapOnICC(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    regType imm_or_reg, regRS1;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;

    int is_trap=0;
    if(e_msg->getEInst()->cond==8){
         //TA cond:1000
        is_trap=1;
    }
    if(e_msg->getEInst()->cond==0){
        //TN
        is_trap=0;
    }
    if(e_msg->getEInst()->cond==9){
        //TNE
        is_trap=~(e_msg->l_PSR_icc_Z);
    }
    if(e_msg->getEInst()->cond==1){
        //TE
        is_trap=(e_msg->l_PSR_icc_Z);
    }
    if(e_msg->getEInst()->cond==10){
        //TG
        is_trap=~(e_msg->l_PSR_icc_Z|(e_msg->l_PSR_icc_N^e_msg->l_PSR_icc_V));
    }
    if(e_msg->getEInst()->cond==2){
        //TLE
        is_trap=(e_msg->l_PSR_icc_Z|(e_msg->l_PSR_icc_N^e_msg->l_PSR_icc_V));
    }
    if(e_msg->getEInst()->cond==11){
        //TGE
        is_trap=~(e_msg->l_PSR_icc_N^e_msg->l_PSR_icc_V);
    }
    if(e_msg->getEInst()->cond==3){
        //TL
        is_trap=(e_msg->l_PSR_icc_N^e_msg->l_PSR_icc_V);
    }
    if(e_msg->getEInst()->cond==12){
        //TGU
        is_trap=~(e_msg->l_PSR_icc_C|e_msg->l_PSR_icc_Z);
    }
    if(e_msg->getEInst()->cond==4){
        //TLEU
        is_trap=(e_msg->l_PSR_icc_C|e_msg->l_PSR_icc_Z);
    }
    if(e_msg->getEInst()->cond==13){
        //TCC
        is_trap=~(e_msg->l_PSR_icc_C);
    }
    if(e_msg->getEInst()->cond==5){
        //TCS
        is_trap=(e_msg->l_PSR_icc_C);
    }
    if(e_msg->getEInst()->cond==14){
        //TPOS
        is_trap=~(e_msg->l_PSR_icc_N);
    }
    if(e_msg->getEInst()->cond==6){
        //TNEG
        is_trap=~(e_msg->l_PSR_icc_N);
    }
    if(e_msg->getEInst()->cond==15){
        //TVC
        is_trap=~(e_msg->l_PSR_icc_V);
    }
    if(e_msg->getEInst()->cond==7){
        //TVS
        is_trap=(e_msg->l_PSR_icc_V);
    }
    if(is_trap==1){
        int setVal = 128 + extract(regRS1+imm_or_reg,0,7);
        int t_tbr = e_msg->l_TBR;
        t_tbr = modifyBits(setVal, t_tbr, TBR_TT_l, TBR_TT_r);
        m_msg->rw_TBR = setVal;
        m_msg->rw_isTBR = true;
        return setVal;
    }

    return RET_SUCCESS;
}

int executeStage::ReadStateRegisterInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    int regRD = e_msg->getEInst()->rd;
    int op3 = e_msg->getEInst()->op3;
    int regRS1 = e_msg->getEInst()->rs1;

    if(op3==40 && regRS1==0){
        //RDY
        int y = e_msg->l_Y;

        m_msg->rw_RD = regRD;
        m_msg->rw_RD_val = y;
        m_msg->rw_isIntReg = true;

    }
    else if(op3==40 && regRS1!=0){
        //RDASR AND STBAR
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}

        int get_Val = e_msg->l_ASR;

        m_msg->rw_RD = regRD;
        m_msg->rw_RD_val = get_Val;
        m_msg->rw_isIntReg = true;
    }
    else if(op3==41){
        //RDPSR
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}

        int get_Val = e_msg->l_PSR;

        m_msg->rw_RD = regRD;
        m_msg->rw_RD_val = get_Val;
        m_msg->rw_isIntReg = true;
    }
    else if(op3==42){
        //RDWIM
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}

        int get_Val = e_msg->l_WIM;

        m_msg->rw_RD = regRD;
        m_msg->rw_RD_val = get_Val;
        m_msg->rw_isIntReg = true;
    }
    else if(op3==43){
        //RDTBR
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}

        int get_Val = e_msg->l_TBR;

        m_msg->rw_RD = regRD;
        m_msg->rw_RD_val = get_Val;
        m_msg->rw_isIntReg = true;
    }

    return RET_SUCCESS;
}

int executeStage::WriteStateRegisterInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
	//TODO delayed writes. see Section B.29 of the sparcv8 specification.
    regType imm_or_reg, regRS1, regRD;
    regRD = e_msg->getEInst()->rd;
    regRS1 = e_msg->l_regRS1;
    imm_or_reg = e_msg->l_reg_or_imm;
    int set_Val = 0;
    if(e_msg->getEInst()->op3==48 && regRD==0){
        //WRY
        set_Val=(imm_or_reg^regRS1);
        m_msg->rw_Y = set_Val;
        m_msg->rw_isY = true;
    }
    if(e_msg->getEInst()->op3==48 && regRD!=0){
        //WRASR
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}
        set_Val=(imm_or_reg^regRS1);

        m_msg->rw_RD = regRD; /// checked no overlap with Int Regs
        m_msg->rw_RD_val = set_Val;
        m_msg->rw_isASR = true;
    }
    if(e_msg->getEInst()->op3==49){
        //WRPSR
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}
        set_Val=(imm_or_reg^regRS1);
        int tmp = (set_Val & 0x0000001F);

        if(getBit(e_msg->l_WIM, tmp) >= (e_msg->l_nWP)){
            cout << "illegal_instruction trap" << endl;
            return illegal_instruction;
        }
        m_msg->rw_PSR = set_Val;
        m_msg->rw_isPSR = true;
    }
    if(e_msg->getEInst()->op3==50){
        //WRWIM
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}

        set_Val=(imm_or_reg^regRS1);
        if(verbose == 2)
        {
        	cout << "write wim=" << set_Val << endl;
        }

        m_msg->rw_WIM = set_Val;
        m_msg->rw_isWIM = true;
    }
    if(e_msg->getEInst()->op3==51){
        //WRTBR
    	if (e_msg->l_PSR_S==0){
			cout << "privileged_instruction exception" << endl;
			return privileged_instruction;
    	}
        set_Val=(imm_or_reg^regRS1);
        m_msg->rw_TBR = set_Val;
        m_msg->rw_isTBR = true;
    }

    return RET_SUCCESS;
}

int executeStage::FpopInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{

    int regFs2 = e_msg->l_regFs2_1;

    if(e_msg->l_PSR_EF==0){
        cout << ("FP disabled trap") <<endl;
        return fp_disabled;
    }

    if(e_msg->getEInst()->opf==196 && e_msg->getEInst()->op3==52){
        //FiTOs
        float regRD;
        int Rd = e_msg->l_FSR_RD;

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
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = *((unsigned int *)&regRD);
        m_msg->rw_isFltReg = true;
    }

    if(e_msg->getEInst()->opf==200 && e_msg->getEInst()->op3==52){
        //FiTOd
        if(is_register_mis_aligned(e_msg->getEInst()->rd))
        {
            cout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }
        double regRD;
        int Rd = e_msg->l_FSR_RD; // Rd - rounding direction

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


        m_msg->rw_F_nWords = 2;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = h_32;
        m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
        m_msg->rw_F_RD2_val = l_32;
        m_msg->rw_isFltReg == true;
    }

    if(e_msg->getEInst()->opf==204 && e_msg->getEInst()->op3==52){
        //FiTOq

        if((e_msg->getEInst()->rd)%4!=0)
        {
            cout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }

        int Rd = e_msg->l_FSR_RD;

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


        m_msg->rw_F_nWords = 4;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = m_1;
        m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
        m_msg->rw_F_RD2_val = m_2;
        m_msg->rw_F_RD3 = e_msg->getEInst()->rd+2;
        m_msg->rw_F_RD3_val = m_3;
        m_msg->rw_F_RD4 = e_msg->getEInst()->rd+3;
        m_msg->rw_F_RD4_val = m_4;
        m_msg->rw_isFltReg = true;
    }

    if(e_msg->getEInst()->opf==209 && e_msg->getEInst()->op3==52){
        //FsTOi
        float num = int_float(regFs2);
        int regD = num;
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = regD;
        m_msg->rw_isFltReg = true;
    }

    if(e_msg->getEInst()->opf==210 && e_msg->getEInst()->op3==52){
        //FdTOi
        if(is_register_mis_aligned(e_msg->getEInst()->rs2))
        {
            cout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }
        int regFs1_lsb = e_msg->l_regFs2_2;
        int regFs1_msb = regFs2;
        long int regFS_tot = ((unsigned long int)regFs1_msb<<32) | ((unsigned int)regFs1_lsb);
        double num = lint_double(regFS_tot);
        cout << "num=" << num << " ,"<<regFS_tot << endl;
        cout << regFs1_msb << ", " << (unsigned int)regFs1_lsb << endl;
        int regD = num;
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = regD;
        m_msg->rw_isFltReg = true;
    }

    if(e_msg->getEInst()->opf==211 && e_msg->getEInst()->op3==52){
        //FqTOi --quad 128 to integer 32
        if((e_msg->getEInst()->rs2)%4!=0)
        {
            cout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }

        int regFs1_1 = e_msg->l_regFs2_1;
        int regFs1_2 = e_msg->l_regFs2_2;
        int regFs1_3 = e_msg->l_regFs2_3;
        int regFs1_4 = e_msg->l_regFs2_4;

        long int FS1_msb = (((long int)regFs1_1)<<32) | regFs1_2;
        long int FS1_lsb = (((long int)regFs1_3)<<32) | regFs1_4;
        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);
        int regD = num1;
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = regD;
        m_msg->rw_isFltReg = true;
    }

    if(e_msg->getEInst()->opf==201 && e_msg->getEInst()->op3==52){
        //FsTOd
        if(e_msg->getEInst()->rd%2!=0){
            cout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }
        float Fs2 = int_float(regFs2);
        double regR = Fs2;
        int msb = *(long int *)&regR;
        int lsb = *(((long int *)&regR)+1);

        m_msg->rw_F_nWords = 2;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = msb;
        m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
        m_msg->rw_F_RD2_val = lsb;
        m_msg->rw_isFltReg = true;

    }

    if(e_msg->getEInst()->opf==205 && e_msg->getEInst()->op3==52){
        //FsTOq
        if(e_msg->getEInst()->rd%4!=0){
            cout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }

        float Fs2 = int_float(regFs2);
        long double regR = Fs2;
        long unsigned int msb = *(long unsigned int *)&regR;
        long unsigned int lsb = *(((long unsigned int *)&regR)+1);


        m_msg->rw_F_nWords = 4;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = msb>>32;
        m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
        m_msg->rw_F_RD2_val = msb;
        m_msg->rw_F_RD3 = e_msg->getEInst()->rd+2;
        m_msg->rw_F_RD3_val = lsb>>32;
        m_msg->rw_F_RD4 = e_msg->getEInst()->rd+3;
        m_msg->rw_F_RD4_val = lsb;
        m_msg->rw_isFltReg = true;

    }

    if(e_msg->getEInst()->opf==198 && e_msg->getEInst()->op3==52){
        //FdTOs
        if(e_msg->getEInst()->rs2%2!=0){
            cout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }

        int regFs1_msb = e_msg->l_regFs2_1;
        int regFs1_lsb = e_msg->l_regFs2_2;

        long int FS1 =  (((unsigned long int)regFs1_msb)<<32) | ((unsigned long int)regFs1_lsb);

        double num = lint_double(FS1);

        int Rd = e_msg->l_FSR_RD;
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
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = *((int *)&regRD);
        m_msg->rw_isFltReg = true;
    }

    if(e_msg->getEInst()->opf==206 && e_msg->getEInst()->op3==52){
        //FdTOq
        if(e_msg->getEInst()->rs2%2!=0 || e_msg->getEInst()->rd%4!=0){
            cout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }

        int regFs1_msb = e_msg->l_regFs2_1;
        int regFs1_lsb = e_msg->l_regFs2_2;

        long int FS1 =  (((unsigned long int)regFs1_msb)<<32) | ((unsigned long int)regFs1_lsb);
        double num = lint_double(FS1);
        long double result = num;

        m_msg->rw_F_nWords = 4;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = *(int *)&result;
        m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
        m_msg->rw_F_RD2_val = *(((int *)&result)+1);
        m_msg->rw_F_RD3 = e_msg->getEInst()->rd+2;
        m_msg->rw_F_RD3_val = *(((int *)&result)+2);
        m_msg->rw_F_RD4 = e_msg->getEInst()->rd+3;
        m_msg->rw_F_RD4_val = *(((int *)&result)+3);
        m_msg->rw_isFltReg = true;
    }

    if((e_msg->getEInst()->opf==199 || e_msg->getEInst()->opf==203) && e_msg->getEInst()->op3==52){
        //FqTOs, FqTOd
        if(e_msg->getEInst()->rs2%4!=0){
            cout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }

        int regFs1_1 = e_msg->l_regFs2_1;
        int regFs1_2 = e_msg->l_regFs2_2;
        int regFs1_3 = e_msg->l_regFs2_3;
        int regFs1_4 = e_msg->l_regFs2_4;

        long int FS1_msb = ((unsigned long int)regFs1_1<<32) | regFs1_2;
        long int FS1_lsb = ((unsigned long int)regFs1_3<<32) | regFs1_4;
        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);

        int Rd = e_msg->l_FSR_RD;

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

        if(e_msg->getEInst()->opf==203){

            if(e_msg->getEInst()->rd%2!=0){
                cout << ("Source is an odd-even register pair float quad instruction") << endl;
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;

                return fp_exception;
            }
            double regRD = static_cast<double>(num1);

            m_msg->rw_F_nWords = 2;
            m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
            m_msg->rw_F_RD1_val = (*(int *)&regRD);
            m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
            m_msg->rw_F_RD2_val = (*(((int *)&regRD)+1));
            m_msg->rw_isFltReg = true;


        }
        else{
            float regRD = static_cast<float>(num1);
            m_msg->rw_F_nWords = 1;
            m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
            m_msg->rw_F_RD1_val = (*(int *)&regRD);
            m_msg->rw_isFltReg = true;

        }

    }


    if(e_msg->getEInst()->opf==1 && e_msg->getEInst()->op3==52){
        //FMOVs
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = regFs2;
        m_msg->rw_isFltReg = true;

    }

    if(e_msg->getEInst()->opf==5 && e_msg->getEInst()->op3==52){
        //FNEGs
        regFs2 = regFs2 ^ (0x80000000);
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = regFs2;
        m_msg->rw_isFltReg = true;

    }

    if(e_msg->getEInst()->opf==9 && e_msg->getEInst()->op3==52){
        //FABSs
        regFs2 = regFs2 & (0x7FFFFFFF);
        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = regFs2;
        m_msg->rw_isFltReg = true;

    }

    if((e_msg->getEInst()->opf==65 || e_msg->getEInst()->opf==69 || e_msg->getEInst()->opf==77) && e_msg->getEInst()->op3==52){
        //FADDs, FSUBs, FDIVs

        int regFs1 = e_msg->l_regFs1_1;
        regFs2 = e_msg->l_regFs2_1;

        float num1 = int_float(regFs1);
        float num2 = int_float(regFs2);
        float result;

        if(e_msg->getEInst()->opf==65){
            result = num1 + num2;
        }

        if(e_msg->getEInst()->opf==69)
        {
            result = num1 - num2;
        }

        if(e_msg->getEInst()->opf==77)
        {
            if(num2 == 0){
                cout << "DIVIDE BY Zero or NaN or Infinity Exception" <<endl;
                return fp_exception;
            }
            result = num1 / num2;
        }

        m_msg->rw_F_nWords = 1;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = *((int *)&result);
        m_msg->rw_isFltReg = true;
    }

    if((e_msg->getEInst()->opf==66 || e_msg->getEInst()->opf==70 || e_msg->getEInst()->opf==78) && e_msg->getEInst()->op3==52){
        //FADDd, FSUBd, FDIVd
        if(is_register_mis_aligned(e_msg->getEInst()->rs1)||is_register_mis_aligned(e_msg->getEInst()->rs2)||is_register_mis_aligned(e_msg->getEInst()->rd))
        {
            cout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }


        int regFs1_msb = e_msg->l_regFs1_1;
        int regFs1_lsb = e_msg->l_regFs1_2;

        long int FS1 = (((unsigned long int)regFs1_msb)<<32) | regFs1_lsb;


        int regFs2_msb = e_msg->l_regFs2_1;
        int regFs2_lsb = e_msg->l_regFs2_2;

        long int FS2 = (((unsigned long int)regFs2_msb)<<32) | regFs2_lsb;

        double num1 = lint_double(FS1);
        double num2 = lint_double(FS2);
        double result;
        if(e_msg->getEInst()->opf==66){
            cout << "num1 ="<<num1 << endl;
            cout << "num2 ="<<num2 << endl;

            result = num1 + num2;
            cout << "result="<<result << endl;
            cout << "result&d="<<&result <<endl;
            cout << "result&i="<<(int *)&result <<endl;
            cout << "result&int="<<*((int *)&result) <<endl;
            cout << "result&int+1="<<*((int *)&result+1) <<endl;
            cout << "result&int+1="<<(((*((long int *)&result)))>>32) <<endl;

        }
        if(e_msg->getEInst()->opf==70)
        {
             result = num1 - num2;
        }
        if(e_msg->getEInst()->opf==78)
        {
            if(num2 == 0){
                cout << "DIVIDE BY ZERO" <<endl;
                return fp_exception;
            }
            result = num1 / num2;
        }
        double result2 = result;
        cout << "result2=" << result2<<" e_msg->getEInst()->rd=" <<e_msg->getEInst()->rd<< endl;

        m_msg->rw_F_nWords = 2;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = *((unsigned long int *)&result2)>>32;
        m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
        m_msg->rw_F_RD2_val = *(((unsigned long int *)&result2));
        m_msg->rw_isFltReg = true;
    }

    if((e_msg->getEInst()->opf==67 || e_msg->getEInst()->opf==71 || e_msg->getEInst()->opf==75 || e_msg->getEInst()->opf==79) && e_msg->getEInst()->op3==52){
        //FADDq, FSUBq, FMULq, FDIVq

        if((e_msg->getEInst()->rs1)%4!=0 || (e_msg->getEInst()->rs2)%4!=0 || (e_msg->getEInst()->rd)%4!=0)
        {
            cout << ("Source is an odd-even register pair float quad instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }


        int regFs1_1 = e_msg->l_regFs1_1;
        int regFs1_2 = e_msg->l_regFs1_2;
        int regFs1_3 = e_msg->l_regFs1_3;
        int regFs1_4 = e_msg->l_regFs1_4;

        long int FS1_msb = ((unsigned long int)regFs1_1<<32) | regFs1_2;
        long int FS1_lsb = ((unsigned long int)regFs1_3<<32) | regFs1_4;


        int regFs2_1 = e_msg->l_regFs2_1;
        int regFs2_2 = e_msg->l_regFs2_2;
        int regFs2_3 = e_msg->l_regFs2_3;
        int regFs2_4 = e_msg->l_regFs2_4;

        long int FS2_msb = ((unsigned long int)regFs2_1<<32) | regFs1_2;
        long int FS2_lsb = ((unsigned long int)regFs2_3<<32) | regFs1_4;

        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);
        long double num2 = llint_ldouble(FS2_msb, FS2_lsb);
        long double result;

        if(e_msg->getEInst()->opf==67){
             result = num1 + num2;
        }

        if(e_msg->getEInst()->opf==71){
            long double result = num1 - num2;
        }

        if(e_msg->getEInst()->opf==75){
             result = num1 * num2;
        }

        if(e_msg->getEInst()->opf==79){
            if(num2 == 0){
                cout << "DIVIDE BY ZERO" <<endl;
                return fp_exception;
            }
            result = num1 / num2;
        }

        long double result2 = result;


        m_msg->rw_F_nWords = 4;
        m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
        m_msg->rw_F_RD1_val = *((int *)&result2);
        m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
        m_msg->rw_F_RD2_val = *(((int *)&result2)+1);
        m_msg->rw_F_RD3 = e_msg->getEInst()->rd+2;
        m_msg->rw_F_RD3_val = *(((int *)&result2)+2);
        m_msg->rw_F_RD4 = e_msg->getEInst()->rd+3;
        m_msg->rw_F_RD4_val = *(((int *)&result2)+3);
        m_msg->rw_isFltReg = true;

    }

    if((e_msg->getEInst()->opf==73 || e_msg->getEInst()->opf==105) && e_msg->getEInst()->op3==52){
        //FMULs, FsMULd

        int regFs1 = e_msg->l_regFs1_1;
        regFs2 = e_msg->l_regFs2_1;

        float num1 = int_float(regFs1);
        float num2 = int_float(regFs2);

        if(e_msg->getEInst()->opf==73){
            float result = num1 * num2;
            m_msg->rw_F_nWords = 1;
            m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
            m_msg->rw_F_RD1_val = *(int *)&result;
            m_msg->rw_isFltReg = true;

        }

        else
        {
            if(is_register_mis_aligned(e_msg->getEInst()->rd))
            {
                cout << ("Source is an odd-even register pair float double instruction") << endl;
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;

                return fp_exception;
            }

            double result = (double)num1 * (double)num2;

            m_msg->rw_F_nWords = 2;
            m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
            m_msg->rw_F_RD1_val = *((unsigned long int *)&result)>>32;
            m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
            m_msg->rw_F_RD2_val = *((unsigned long int *)&result);
            m_msg->rw_isFltReg = true;

        }

    }

    if((e_msg->getEInst()->opf==74 || e_msg->getEInst()->opf==110) && e_msg->getEInst()->op3==52){
        //FMULd, FdMULq
        if(is_register_mis_aligned(e_msg->getEInst()->rs1)||is_register_mis_aligned(e_msg->getEInst()->rs2)||is_register_mis_aligned(e_msg->getEInst()->rd))
        {
            cout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }


        int regFs1_msb = e_msg->l_regFs1_1;
        int regFs1_lsb = e_msg->l_regFs1_2;

        long int FS1 = (((long int)regFs1_msb)<<32) | regFs1_lsb;


        int regFs2_msb = e_msg->l_regFs2_1;
        int regFs2_lsb = e_msg->l_regFs2_2;

        long int Fs2 = (((long int)regFs2_msb)<<32) | regFs2_lsb;

        double num1 = lint_double(FS1);
        double num2 = lint_double(Fs2);
        if(e_msg->getEInst()->opf==74){
            double result = num1 * num2;

            m_msg->rw_F_nWords = 2;
            m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
            m_msg->rw_F_RD1_val = *((unsigned long int *)&result)>>32;
            m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
            m_msg->rw_F_RD2_val = *((unsigned long int *)&result);
            m_msg->rw_isFltReg = true;

        }
        else
        {
            if((e_msg->getEInst()->rd)%4!=0){
                cout << ("Source is an odd-even register pair float double instruction") << endl;
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;

                return fp_exception;
            }

            long double result = num1 * num2;
            long double result2 = result;

            m_msg->rw_F_nWords = 4;
            m_msg->rw_F_RD1 = e_msg->getEInst()->rd;
            m_msg->rw_F_RD1_val = *((int *)&result2);
            m_msg->rw_F_RD2 = e_msg->getEInst()->rd+1;
            m_msg->rw_F_RD2_val = *(((int *)&result2)+1);
            m_msg->rw_F_RD3 = e_msg->getEInst()->rd+2;
            m_msg->rw_F_RD3_val = *(((int *)&result2)+2);
            m_msg->rw_F_RD4 = e_msg->getEInst()->rd+3;
            m_msg->rw_F_RD4_val = *(((int *)&result2)+3);
            m_msg->rw_isFltReg = true;
        }
    }

    if((e_msg->getEInst()->opf==81 || e_msg->getEInst()->opf==85) && e_msg->getEInst()->op3==53){
        //FCMPs, FCMPEs

        int regFs1 = e_msg->l_regFs1_1;
        regFs2 =e_msg->l_regFs2_1;

        float num1 = int_float(regFs1);
        float num2 = int_float(regFs2);
        if(num1==num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }
        if(num1<num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }
        if(num1>num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }

        if(is_nan_int(regFs1)!=0 || is_nan_int(regFs2)!=0){
            //Nan!

            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(3, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            if(e_msg->getEInst()->opf==85){
                cout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
            else if(is_nan_int(regFs1)==1 || is_nan_int(regFs2)==1){
                cout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
        }
        if(is_infinity_int(regFs1)!=0 || is_infinity_int(regFs2)!=0){

            int num1 = is_infinity_int(regFs1);
            int num2 = is_infinity_int(regFs2);

            if(num1==num2){
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;
            }
            if(num1<num2){
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;
            }
            if(num1>num2){
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;
            }
        }
    }

    if((e_msg->getEInst()->opf==82 || e_msg->getEInst()->opf==86) && e_msg->getEInst()->op3==53){
        //FCMPd, FCMPEd
        if(is_register_mis_aligned(e_msg->getEInst()->rs1)||is_register_mis_aligned(e_msg->getEInst()->rs2))
        {
            cout << ("Source is an odd-even register pair float double instruction") << endl;
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }


        int regFs1_msb = e_msg->l_regFs1_1;
        int regFs1_lsb = e_msg->l_regFs1_2;

        long int FS1 = ((long int)regFs1_msb<<32) | regFs1_lsb;

        int regFs2_msb = e_msg->l_regFs2_1;
        int regFs2_lsb = e_msg->l_regFs2_2;

        long int FS2 = ((long int)regFs2_msb<<32) | regFs2_lsb;

        double num1 = lint_double(FS1);
        double num2 = lint_double(FS2);

        if(num1==num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }
        if(num1<num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }
        if(num1>num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }

        if(is_nan_lint(FS1)!=0 || is_nan_lint(FS2)!=0){
            //Nan!

            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(3, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            if(e_msg->getEInst()->opf==86){
                cout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
            else if(is_nan_lint(FS1)==1 || is_nan_lint(FS2)==1){
                cout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
        }

        if(is_infinity_lint(FS1)!=0 || is_infinity_lint(FS2)!=0){

            int num1 = is_infinity_lint(FS1);
            int num2 = is_infinity_lint(FS2);

            if(num1==num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
            }
            if(num1<num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
            }
            if(num1>num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
            }
        }

    }

    if((e_msg->getEInst()->opf==83 || e_msg->getEInst()->opf==87) && e_msg->getEInst()->op3==53){
        //FCMPq, FCMPEq,

        if((e_msg->getEInst()->rs1)%4!=0 || (e_msg->getEInst()->rs2)%4!=0)
        {
            cout << ("Source is an odd-even register pair float quad instruction") << endl;
                // sregister->SRegisters.fsr->setFtt(INVALID_FP_REGISTER);
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            return fp_exception;
        }


        int regFs1_1 = e_msg->l_regFs1_1;
        int regFs1_2 = e_msg->l_regFs1_2;
        int regFs1_3 = e_msg->l_regFs1_3;
        int regFs1_4 = e_msg->l_regFs1_4;

        long int FS1_msb = ((long int)regFs1_1<<32) | regFs1_2;
        long int FS1_lsb = ((long int)regFs1_3<<32) | regFs1_4;


        int regFs2_1 = e_msg->l_regFs2_1;
        int regFs2_2 = e_msg->l_regFs2_2;
        int regFs2_3 = e_msg->l_regFs2_3;
        int regFs2_4 = e_msg->l_regFs2_4;

        long int FS2_msb = ((long int)regFs2_1<<32) | regFs1_2;
        long int FS2_lsb = ((long int)regFs2_3<<32) | regFs1_4;

        long double num1 = llint_ldouble(FS1_msb, FS1_lsb);
        long double num2 = llint_ldouble(FS2_msb, FS2_lsb);
        if(num1==num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }
        if(num1<num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }
        if(num1>num2){
            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;
        }

        if(is_nan_llint(FS1_msb, FS1_lsb)!=0 || is_nan_llint(FS2_msb,FS2_lsb)!=0){
            //Nan!

            int t_fsr = e_msg->l_FSR;
            t_fsr = modifyBits(3, t_fsr, FSR_fcc_l, FSR_fcc_r);
            m_msg->rw_FSR = t_fsr;
            m_msg->rw_isFSR = true;

            if(e_msg->getEInst()->opf==87){
                cout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
            else if(is_nan_llint(FS1_msb, FS1_lsb)==1 || is_nan_llint(FS2_msb,FS2_lsb)==1){
                cout << "INVALID EXCEPTION" << endl;
                return fp_exception;
            }
        }

        if(is_infinity_llint(FS1_msb, FS1_lsb)!=0 || is_infinity_llint(FS2_msb,FS2_lsb)!=0){

            int num1 = is_infinity_llint(FS1_msb, FS1_lsb);
            int num2 = is_infinity_llint(FS2_msb,FS2_lsb);

            if(num1==num2){
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(0, t_fsr, FSR_fcc_l, FSR_fcc_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;
            }
            if(num1<num2){
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(1, t_fsr, FSR_fcc_l, FSR_fcc_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;
            }
            if(num1>num2){
                int t_fsr = e_msg->l_FSR;
                t_fsr = modifyBits(2, t_fsr, FSR_fcc_l, FSR_fcc_r);
                m_msg->rw_FSR = t_fsr;
                m_msg->rw_isFSR = true;
            }
        }


    }

    if(e_msg->getEInst()->opf==41 || e_msg->getEInst()->opf==42 || e_msg->getEInst()->opf==43){
        //FSQRTs, FSQRTd, FSQRTq

        // if(is_register_mis_aligned(e_msg->getEInst()->rs1)||is_register_mis_aligned(e_msg->getEInst()->rs2)||is_register_mis_aligned(e_msg->getEInst()->rd))
        // {
        //     cout << ("Source is an odd-even register pair float double instruction") << endl;
        //     sregister->SRegisters.fsr->setFtt(INVALID_FP_REGISTER);
        //     return fp_exception;
        // }
    }

    return RET_SUCCESS;


}



/*
 * Updates Integer Condition Code (ICC) bits based on the result of addition. op==0 for add, op==1 for subtract
 */
void executeStage::updateICCAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int op, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    unsigned int  t_psr = e_msg->l_PSR;

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


    m_msg->rw_PSR = t_psr;
    m_msg->rw_isPSR = true;
}


/*
 * Updates Integer Condition Code (ICC) bits based on the result of tagged addition and subtraction.
 * If isTVOpcode = 1, the opcodes will be treated as TADDCCTV and TSUBCCTV.
 * If isTVOpcode = 0, the opcodes will be treated as TADDCC and TSUBCC.
 * Returns 1, if tagged overflow has occurred.
 */
//op=0 for add
int executeStage::taggedAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int isTVOpcode, int op, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
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

        updateICCAddSubtract(regRS1,reg_or_imm, regRD, op, e_msg, m_msg);
        int t_psr=e_msg->l_PSR;
        if(isTaggedOverflow==1) {
            t_psr = modifyBits(1, t_psr, 21, 21);
            }
        else {
            t_psr = modifyBits(0, t_psr, 21, 21);

        }
        m_msg->rw_PSR = t_psr;
        m_msg->rw_isPSR = true;

    }
    return isTaggedOverflow;
}

/*
 * Updates Integer Condition Code (ICC) bits based on the result of multiplication and logical operations.
 */
void executeStage::updateICCMulLogical(regType regRD, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
	int signBit_regRD = getBit(regRD, SIGN_BIT);

    int t_psr=e_msg->l_PSR;

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

    m_msg->rw_PSR = t_psr;
    m_msg->rw_isPSR = true;
}

/*
 * Updates Integer Condition Code (ICC) bits based on the result of division.
 */
void executeStage::updateICCDiv(regType regRD, int isOverflow, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg)
{
    int t_psr = e_msg->l_PSR;
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

    m_msg->rw_PSR = t_psr;
    m_msg->rw_isPSR = true;

}

void executeStage::simulateOneCycle()
{
	bool anyMulticycleExecutionCompletion = false;
	while(m_eventQueue->empty() == false
				&& m_eventQueue->top()->getEventTime() <= getClock()
				&& m_executeStage_memoryStage_interface->getBusy() == false)
	{
		m_eventQueue->pop();
		executeInstruction();
		m_multicycleExecutionInProgress = false;
		anyMulticycleExecutionCompletion = true;
	}

	if(anyMulticycleExecutionCompletion == false
			&& m_multicycleExecutionInProgress == false
			&& m_registerAccessStage_executeStage_interface->doesElementHaveAnyPendingMessage(this) == true
				&& m_executeStage_memoryStage_interface->getBusy() == false)
	{
		registerAccessStageexecuteStageMessage* e_msg = (registerAccessStageexecuteStageMessage*) (m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(this));
		instruction* e_inst = e_msg->getEInst();
		addrType e_pc = e_msg->getEPC();

		if(e_inst->op == 2
				&&  (e_inst->op3 == 36 /*MULScc*/
						|| e_inst->op3 == 10 || e_inst->op3 == 11 || e_inst->op3 == 26 || e_inst->op3 == 27 /*multiply*/))
		{
			m_eventQueue->push(new executioncompleteevent(getClock() + multiplyLatency - 1));
			m_multicycleExecutionInProgress = true;
		}
		else if(e_inst->op == 2
				&&  (e_inst->op3 == 14 || e_inst->op3 == 15 || e_inst->op3 == 30 || e_inst->op3 == 31 /*divide*/))
		{
			m_eventQueue->push(new executioncompleteevent(getClock() + divideLatency - 1));
			m_multicycleExecutionInProgress = true;
		}
		else
		{
			executeInstruction();
			m_multicycleExecutionInProgress = false;
		}
	}

	if((m_executeStage_memoryStage_interface->getBusy() == true
			&& m_registerAccessStage_executeStage_interface->doesElementHaveAnyPendingMessage(this) == true)
			|| m_multicycleExecutionInProgress == true)
	{
		m_registerAccessStage_executeStage_interface->setBusy(true);
	}
	else
	{
		m_registerAccessStage_executeStage_interface->setBusy(false);
	}
}

void executeStage::executeInstruction()
{
	if(m_registerAccessStage_executeStage_interface->doesElementHaveAnyPendingMessage(this) == true && m_hasTrapOccurred == true)
	{
		registerAccessStageexecuteStageMessage* e_msg = (registerAccessStageexecuteStageMessage*) (m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(this)); //note we peek here. busy is set to true if it is a memory instruction. we pop when the response arrives. busy is set to false then.
		addrType e_pc = e_msg->getEPC();
		if(e_pc == m_containingCore->getExceptionStage()->getNewPCAfterTrap())
		{
			m_hasTrapOccurred = false;
		}
		else
		{
			//assuming there can be only one pending message
			m_registerAccessStage_executeStage_interface->popElementsPendingMessage(this);
			delete e_msg;
			m_multicycleExecutionInProgress = false;
			return;
		}
	}

	if(m_registerAccessStage_executeStage_interface->doesElementHaveAnyPendingMessage(this) == true
			&& m_executeStage_memoryStage_interface->getBusy() == false)
	{
		registerAccessStageexecuteStageMessage* e_msg = (registerAccessStageexecuteStageMessage*) (m_registerAccessStage_executeStage_interface->popElementsPendingMessage(this));
		instruction* e_inst = e_msg->getEInst();
		addrType e_pc = e_msg->getEPC();
		//std::cout << dec << "[" << getClock() << "] EX COMPLETE : " << hex << e_pc << " : " << e_inst->instructionWord << dec << "\n";

		executeStagememoryStageMessage* m_msg = new executeStagememoryStageMessage(this, m_memoryStage, e_inst, e_pc);

		int returnValue = RET_SUCCESS;

		if(e_msg->getEInst()->op==1){
			CallInstruction(e_msg, m_msg);//Checked correct
		}
		else if(e_msg->getEInst()->op==0){
			//op2
			//0 - UNIMP
			//1 - unnimplemented
			//3 - unnimplemented
			//5 - unnimplemented
			//7 - CBcc
			if(e_msg->getEInst()->op2 == 4){
				// SethiNop(ins, sregister, memory); //checked correct
				SethiNop(e_msg, m_msg); //checked correct
			}
			else{
				if(e_msg->getEInst()->op2==2)
					BranchIntegerInstructions(e_msg, m_msg); //EVERYTHING HERE IS DONE EXCEPT THAT BICC SHOULD NOT BE PLACED IN DELAY SLOT OF CONDITIONAL BRANCH
				else if(e_msg->getEInst()->op2 == 6){
					returnValue = BranchFloatInstructions(e_msg, m_msg);
				}

			}
		}
		else{
			if(e_msg->getEInst()->op==3){ //memory instructions
				if(e_msg->getEInst()->op3==0 || e_msg->getEInst()->op3==1 || e_msg->getEInst()->op3==2 ||e_msg->getEInst()->op3==3 ||e_msg->getEInst()->op3==9 ||e_msg->getEInst()->op3==10 )
					returnValue = LoadIntegerInstructions(e_msg, m_msg); //PRIVILEDGED LEFT
				else if(e_msg->getEInst()->op3==32 || e_msg->getEInst()->op3==33 || e_msg->getEInst()->op3==35 )
					returnValue = LoadFloatingPointInstructions(e_msg, m_msg); //LDFSR ins LEFT,<< >> ISSUE
				else if(e_msg->getEInst()->op3==4 || e_msg->getEInst()->op3==5 || e_msg->getEInst()->op3==6 || e_msg->getEInst()->op3==7 )
					returnValue=StoreIntegerInstructions(e_msg, m_msg); // PSR, PRIVILEDGED INSTRUCTIONS LEFT
				else if(e_msg->getEInst()->op3==36 || e_msg->getEInst()->op3==39 || e_msg->getEInst()->op3==37 || e_msg->getEInst()->op3==38)
					returnValue=StoreFloatInstructions(e_msg, m_msg); //STDFQ Left
				else if(e_msg->getEInst()->op3 == 13 || e_msg->getEInst()->op3 == 29){
					returnValue=AtomicLoadStoreUnsignedByte(e_msg, m_msg);//LDSTUB S bit in PSR not checked, LDSTUBA is left
				}
				else{
					returnValue=SWAP(e_msg, m_msg); //PSR, PRIVILEDGED LEFT
				}

			}
			//arithmetic operations
			else{ //IN THIS CASE OP==2

					if(e_msg->getEInst()->op3==56)
						returnValue=JumpAndLink(e_msg, m_msg); //SEEMS OKAY

					else if (e_msg->getEInst()->op3==57)
						returnValue = RETT(e_msg, m_msg); //NOT COMPLETE YET THE ERROR_MODE NOT IMPLEMENTED

					else{

							if(e_msg->getEInst()->op3==1 || e_msg->getEInst()->op3==17 || e_msg->getEInst()->op3==5 ||e_msg->getEInst()->op3==21 ||e_msg->getEInst()->op3==2 ||e_msg->getEInst()->op3==18
							||e_msg->getEInst()->op3==6 ||e_msg->getEInst()->op3==22 ||e_msg->getEInst()->op3==3 ||e_msg->getEInst()->op3==19 ||e_msg->getEInst()->op3==7||e_msg->getEInst()->op3==23)
								LogicalInstructions(e_msg, m_msg); //CHECKED, CORRECT PERFECT

							else if(e_msg->getEInst()->op3==58)
								returnValue=TrapOnICC(e_msg, m_msg); //SOME DISABLES TRAP DECREMENT CWP etc left.

							else if(e_msg->getEInst()->op3==40 || e_msg->getEInst()->op3==41 || e_msg->getEInst()->op3==42 || e_msg->getEInst()->op3==43)
								ReadStateRegisterInstructions(e_msg, m_msg);

							else if(e_msg->getEInst()->op3==48||e_msg->getEInst()->op3==49||e_msg->getEInst()->op3==50)
								WriteStateRegisterInstructions(e_msg, m_msg);

							else if(e_msg->getEInst()->op3==52 || e_msg->getEInst()->op3==53){
								FpopInstructions(e_msg, m_msg);
							}

							else if(e_msg->getEInst()->op3 == 37 || e_msg->getEInst()->op3 == 38 || e_msg->getEInst()->op3 == 39)
								ShiftInstructions(e_msg, m_msg);

							else if(e_msg->getEInst()->op3 == 0 || e_msg->getEInst()->op3 == 16 || e_msg->getEInst()->op3 == 8 || e_msg->getEInst()->op3 == 24)
								AddInstructions(e_msg, m_msg); //CHECKED CORRECT PERFECT

							else if(e_msg->getEInst()->op3 == 32 || e_msg->getEInst()->op3 == 34)
								returnValue = TaggedAddInstructions(e_msg, m_msg);//CHECKED seems CORRECT

							else if(e_msg->getEInst()->op3 == 4 || e_msg->getEInst()->op3 == 20 || e_msg->getEInst()->op3 == 12 || e_msg->getEInst()->op3 == 28)
								SubtractInstructions(e_msg, m_msg); //Checked Correct Perfect

							else if(e_msg->getEInst()->op3 == 33 || e_msg->getEInst()->op3 == 35)
								returnValue = TaggedSubtractInstructions(e_msg, m_msg);///Checked seems correct

							else if(e_msg->getEInst()->op3 == 36)
								MultiplyStepInstruction(e_msg, m_msg);//Checked, correct

							else if(e_msg->getEInst()->op3 == 10 || e_msg->getEInst()->op3 == 11 || e_msg->getEInst()->op3 == 26 || e_msg->getEInst()->op3 == 27)
								MultiplyInstructions(e_msg, m_msg);//Checked, correct

							else if(e_msg->getEInst()->op3 == 14 || e_msg->getEInst()->op3 == 30 || e_msg->getEInst()->op3 == 15 || e_msg->getEInst()->op3 == 31)
								returnValue = DivideInstructions(e_msg, m_msg); //SDIV AND SDIVcc done but doubt in their overflow!!

							else if(e_msg->getEInst()->op3 == 60 || e_msg->getEInst()->op3 == 61)
								returnValue= SaveAndRestoreInstructions(e_msg, m_msg); //checked PERFECT!!
						}
				}
		}

		m_msg->xc_isError = e_msg->xc_isError;
		m_msg->xc_psr = e_msg->xc_psr;
		m_msg->xc_tbr = e_msg->xc_tbr;
		m_msg->xc_nWP = e_msg->xc_nWP;
		m_msg->xc_PC = e_msg->xc_PC;
		m_msg->xc_nPC = e_msg->xc_nPC;

		m_msg->retVal = returnValue;
		m_executeStage_memoryStage_interface->addPendingMessage(m_msg);
		delete e_msg; //remember to delete messages and events once their work is done!
	}
}

void executeStage::setHasTrapOccurred()
{
	m_hasTrapOccurred = true;
}

std::string* executeStage::getStatistics()
{
	return 0;
}

core* executeStage::getContainingCore()
{
	return m_containingCore;
}

void executeStage::setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface)
{
	m_registerAccessStage_executeStage_interface = x_registerAccessStage_executeStage_interface;
}

void executeStage::setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface)
{
	m_executeStage_memoryStage_interface = x_executeStage_memoryStage_interface;
}

counterType executeStage::getNumberOfBranches()
{
	return pmc_branch;
}

counterType executeStage::getNumberOfTakenBranches()
{
	return pmc_takenBranch;
}

executeStagememoryStageMessage::executeStagememoryStageMessage(element* x_producer, element* x_consumer, instruction* x_m_inst, unsigned int x_m_pc) : message(x_producer, x_consumer)
{
	m_m_inst = x_m_inst;
	m_m_pc = x_m_pc;

	isMemIns = false;
	retVal = 0;

	m_addr = 0;
	isInt = false;
	isFloat = false;
	isAtomic = false;
	isLoad = false;
	isStore = false;
	isSwap = false;

	/// to Store
	regRD = 0;
	regNextRD = 0;

	FSR = 0;

	//for the XC stage
	xc_isError = false;
	xc_psr = 0;
	xc_tbr = 0;
	xc_nWP = 0;
	xc_PC = 0;
	xc_nPC = 0;

	//for the RW stage
	rw_isIntReg = false;
	rw_isFltReg = false;
	rw_isASR = false;
	rw_isPSR = false;
	rw_isFSR = false;
	rw_isTBR = false;
	rw_isY = false;
	rw_isWIM = false;
	rw_isControl = false;
	rw_isTrapSaveCounter = false;
	    /////////////////////////////
	rw_RD = 0;
	rw_RD_val = 0;
	rw_F_nWords = 0;
	rw_F_RD1 = 0;
	rw_F_RD2 = 0;
	rw_F_RD3 = 0;
	rw_F_RD4 = 0;
	rw_F_RD1_val = 0;
	rw_F_RD2_val = 0;
	rw_F_RD3_val = 0;
	rw_F_RD4_val = 0;
	rw_PC = 0;
	rw_nPC = 0;
	rw_PSR = 0;
	rw_FSR = 0;
	rw_TBR = 0;
	rw_Y = 0;
	rw_WIM = 0;
}

executeStagememoryStageMessage::~executeStagememoryStageMessage()
{

}

instruction* executeStagememoryStageMessage::getMInst()
{
	return m_m_inst;
}

unsigned int executeStagememoryStageMessage::getMPC()
{
	return m_m_pc;
}

executioncompleteevent::executioncompleteevent(clockType x_eventTime)
{
	setEventTime(x_eventTime);
}

executioncompleteevent::~executioncompleteevent()
{

}

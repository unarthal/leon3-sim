#include "architecture/processor/core/exceptionStage.h"
#include "architecture/processor/core/core.h"
#include "architecture/processor/core/memoryStage.h"
#include "architecture/processor/core/executeStage.h"
#include "architecture/processor/core/registerAccessStage.h"
#include "architecture/processor/core/decodeStage.h"
#include "architecture/processor/core/fetchStage.h"
#include "architecture/system.h"
#include <iostream>
#include "generic/utility.h"
#include "architecture/processor/processor.h"

exceptionStage::exceptionStage(core* x_containingCore)
{
	m_containingCore = x_containingCore;
	m_memoryStage_exceptionStage_interface = 0;
	m_exceptionStage_writebackStage_interface = 0;
	m_writebackStage = m_containingCore->getWritebackStage();
	m_newPCAfterTrap = 0x40000000;
}

exceptionStage::~exceptionStage()
{

}

extern int verbose;

void rw_copy(memoryStageexceptionStageMessage* x_msg, exceptionStagewritebackStageMessage* w_msg);

int exceptionStage::trap(int memAddr, memoryStageexceptionStageMessage* x_x_msg, exceptionStagewritebackStageMessage* x_w_msg)
{
    int t_psr=x_x_msg->xc_psr;
    int t_tbr=x_x_msg->xc_tbr;

    unsigned int ET = getBits(x_x_msg->xc_psr,0, PSR_ET_l, PSR_ET_r);
    unsigned int S = getBits(x_x_msg->xc_psr,0, PSR_S_l, PSR_S_r);
    unsigned int CWP = getBits(x_x_msg->xc_psr, 0, PSR_CWP_l, PSR_CWP_r);

    if(ET==0)
    {
        return error_mode;
    }

    t_psr = modifyBits(0, t_psr, PSR_ET_l, PSR_ET_r);
    t_psr = modifyBits(S, t_psr, PSR_PS_l, PSR_PS_r);
    t_psr = modifyBits(1, t_psr, PSR_S_l, PSR_S_r);

    //cout << "intrap getoldcwp:" <<CWP<< endl;

    t_psr = modifyBits( ((CWP-1)%x_x_msg->xc_nWP), t_psr, PSR_CWP_l, PSR_CWP_r);
    if (CWP < 0)
    {
        t_psr = modifyBits( (CWP + x_x_msg->xc_nWP), t_psr, PSR_CWP_l, PSR_CWP_r);
    }
    x_w_msg->rw_PSR = t_psr;
    x_w_msg->rw_isPSR = true;

    //cout << "intrap getnewcwp:" <<CWP << endl;

    t_tbr = modifyBits(memAddr, t_tbr, TBR_TT_l, TBR_TT_r);
    x_w_msg->rw_TBR = t_tbr;
    x_w_msg->rw_isTBR = true;

    x_w_msg->rw_PC = x_x_msg->getXPC();
    x_w_msg->rw_nPC = x_x_msg->getXPC() + 4;
    //x_w_msg->rw_PC = t_tbr;
	//x_w_msg->rw_nPC = t_tbr;
	x_w_msg->rw_isControl = true;
    x_w_msg->rw_isTrapSaveCounter=true;

    //flush all previous latches
    m_newPCAfterTrap = t_tbr;
    m_containingCore->getMemoryStage()->setHasTrapOccurred();
    m_containingCore->getExecuteStage()->setHasTrapOccurred();
    m_containingCore->getRegisterAccessStage()->setHasTrapOccurred();
    m_containingCore->getDecodeStage()->setHasTrapOccurred();
    m_containingCore->getFetchStage()->setHasTrapOccurred();

    if(verbose == 2)
    {
		cout << "Trap generated" << endl;
		cout << "TRAP PC:" << hex << x_x_msg->getXPC() << dec << endl;
		cout << "new PC = t_tbr = " << hex << t_tbr << dec << "\n";
    }
    return RET_SUCCESS;
}

void exceptionStage::simulateOneCycle()
{
	if(m_memoryStage_exceptionStage_interface->doesElementHaveAnyPendingMessage(this) == true
			&& m_exceptionStage_writebackStage_interface->getBusy() == false)
	{
		memoryStageexceptionStageMessage* x_msg = (memoryStageexceptionStageMessage*) m_memoryStage_exceptionStage_interface->popElementsPendingMessage(this);
		instruction* x_inst = x_msg->getXInst();
		addrType x_pc = x_msg->getXPC();
		//std::cout << dec << "[" << getClock() << "] X RECV : " << hex << x_pc << " : " << x_inst->instructionWord << dec << "\n";

		exceptionStagewritebackStageMessage* w_msg = new exceptionStagewritebackStageMessage(this, m_writebackStage, x_inst, x_pc);

	    int retval = x_msg->retVal;
	    if(retval != 1)
	    {
	    	int is_error=trap(retval, x_msg, w_msg);
	        if(is_error == -1)
	        {
	        	//TODO detailing required. trap masking, deferred traps, etc.
	        	showErrorAndExit("processor entering error mode. trap occurred when ET=0\n");
	        }
	    }
	    else
	    {
	        // No Traps
	        rw_copy(x_msg, w_msg);
	    }

        m_exceptionStage_writebackStage_interface->addPendingMessage(w_msg);
        delete x_msg;
	}

	if(m_exceptionStage_writebackStage_interface->getBusy() == true
			&& m_memoryStage_exceptionStage_interface->doesElementHaveAnyPendingMessage(this) == true)
	{
		m_memoryStage_exceptionStage_interface->setBusy(true);
	}
	else
	{
		m_memoryStage_exceptionStage_interface->setBusy(false);
	}
}

addrType exceptionStage::getNewPCAfterTrap()
{
	return m_newPCAfterTrap;
}

std::string* exceptionStage::getStatistics()
{
	return 0;
}

core* exceptionStage::getContainingCore()
{
	return m_containingCore;
}

void exceptionStage::setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface)
{
	m_memoryStage_exceptionStage_interface = x_memoryStage_exceptionStage_interface;
}

void exceptionStage::setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface)
{
	m_exceptionStage_writebackStage_interface = x_exceptionStage_writebackStage_interface;
}

exceptionStagewritebackStageMessage::exceptionStagewritebackStageMessage(element* x_producer, element* x_consumer, instruction* x_w_inst, unsigned int x_w_pc) : message(x_producer, x_consumer)
{
	m_w_inst = x_w_inst;
	m_w_pc = x_w_pc;

	isregWrite = false;
	retVal = 0;

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

exceptionStagewritebackStageMessage::~exceptionStagewritebackStageMessage()
{

}

instruction* exceptionStagewritebackStageMessage::getWInst()
{
	return m_w_inst;
}

unsigned int exceptionStagewritebackStageMessage::getWPC()
{
	return m_w_pc;
}

void rw_copy(memoryStageexceptionStageMessage* x_msg, exceptionStagewritebackStageMessage* w_msg)
{
	w_msg->rw_isIntReg = x_msg->rw_isIntReg;
	w_msg->rw_isFltReg = x_msg->rw_isFltReg;
	w_msg->rw_isASR = x_msg->rw_isASR;
	w_msg->rw_isPSR = x_msg->rw_isPSR;
	w_msg->rw_isFSR = x_msg->rw_isFSR;
	w_msg->rw_isTBR = x_msg->rw_isTBR;
	w_msg->rw_isY = x_msg->rw_isY;
	w_msg->rw_isWIM = x_msg->rw_isWIM;
	w_msg->rw_isControl = x_msg->rw_isControl;
	w_msg->rw_isTrapSaveCounter = x_msg->rw_isTrapSaveCounter;
		    /////////////////////////////
	w_msg->rw_RD = x_msg->rw_RD;
	w_msg->rw_RD_val = x_msg->rw_RD_val;
	w_msg->rw_nextRD = x_msg->rw_nextRD;
	w_msg->rw_nextRD_val = x_msg->rw_nextRD_val;
	w_msg->rw_F_nWords = x_msg->rw_F_nWords;
	w_msg->rw_F_RD1 = x_msg->rw_F_RD1;
	w_msg->rw_F_RD2 = x_msg->rw_F_RD2;
	w_msg->rw_F_RD3 = x_msg->rw_F_RD3;
	w_msg->rw_F_RD4 = x_msg->rw_F_RD4;
	w_msg->rw_F_RD1_val = x_msg->rw_F_RD1_val;
	w_msg->rw_F_RD2_val = x_msg->rw_F_RD2_val;
	w_msg->rw_F_RD3_val = x_msg->rw_F_RD3_val;
	w_msg->rw_F_RD4_val = x_msg->rw_F_RD4_val;
	w_msg->rw_PC = x_msg->rw_PC;
	w_msg->rw_nPC = x_msg->rw_nPC;
	w_msg->rw_PSR = x_msg->rw_PSR;
	w_msg->rw_FSR = x_msg->rw_FSR;
	w_msg->rw_TBR = x_msg->rw_TBR;
	w_msg->rw_Y = x_msg->rw_Y;
	w_msg->rw_WIM = x_msg->rw_WIM;
}

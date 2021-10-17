#include "architecture/processor/core/writebackStage.h"
#include "architecture/processor/core/core.h"
#include "architecture/processor/core/exceptionStage.h"
#include "architecture/system.h"
#include <iostream>

writebackStage::writebackStage(core* x_containingCore)
{
	m_containingCore = x_containingCore;
	sregister = m_containingCore->getRegisterFile();
	m_exceptionStage_writebackStage_interface = 0;

	pmc_instructions = 0;
}

writebackStage::~writebackStage()
{

}

void writebackStage::simulateOneCycle()
{
	if(m_exceptionStage_writebackStage_interface->doesElementHaveAnyPendingMessage(this) == true)
	{
		exceptionStagewritebackStageMessage* w_msg = (exceptionStagewritebackStageMessage*) m_exceptionStage_writebackStage_interface->popElementsPendingMessage(this);
		instruction* w_inst = w_msg->getWInst();
		addrType w_pc = w_msg->getWPC();
		//std::cout << dec << "[" << getClock() << "] WB RECV : " << hex << w_pc << " : " << w_inst->instructionWord << dec << "\n";

		if (w_msg->rw_isPSR == true)
		{
			sregister->SRegisters->psr->setPSR(w_msg->rw_PSR);
		}

		if(w_msg->rw_isIntReg)
		{
			regType RD = w_msg->rw_RD;
			regType RD_val = w_msg->rw_RD_val;
			sregister->setRegister(RD, RD_val);
			if(w_inst->op == 3 && w_inst->op3 == 3/*LDD*/)
			{
				sregister->setRegister(w_msg->rw_nextRD, w_msg->rw_nextRD_val);
			}
		}

		if(w_msg->rw_isFltReg == true)
		{
			int nWords = w_msg->rw_F_nWords;
			switch (nWords)
			{
				case 1: //word
					sregister->setFloatingRegister (w_msg->rw_F_RD1, w_msg->rw_F_RD1_val);
					break;

				case 2: //double word
					sregister->setFloatingRegister(w_msg->rw_F_RD1, w_msg->rw_F_RD1_val);
					sregister->setFloatingRegister(w_msg->rw_F_RD2,	w_msg->rw_F_RD2_val);
					break;

				case 4: //quad word
					sregister->setFloatingRegister(w_msg->rw_F_RD1, w_msg->rw_F_RD1_val);
					sregister->setFloatingRegister(w_msg->rw_F_RD2, w_msg->rw_F_RD2_val);
					sregister->setFloatingRegister(w_msg->rw_F_RD3, w_msg->rw_F_RD3_val);
					sregister->setFloatingRegister(w_msg->rw_F_RD4, w_msg->rw_F_RD4_val);
					break;

				default:
					cout << "Unknown no. of Words to write in FPU regs:" << nWords << "\n";
					break;
			}
		}

		if(w_msg->rw_isControl == true)
		{
			sregister->setPC(w_msg->rw_PC);
			sregister->setnPC(w_msg->rw_nPC);
		}

		if (w_msg->rw_isFSR == true)
		{
			sregister->SRegisters->fsr->setFSR(w_msg->rw_FSR);
		}

		if (w_msg->rw_isTBR == true)
		{
			sregister->SRegisters->tbr->setTbr(w_msg->rw_TBR);
		}

		if (w_msg->rw_isY == true)
		{
			sregister->setY(w_msg->rw_Y);
		}

		if (w_msg->rw_isWIM == true)
		{
			sregister->SRegisters->wim = w_msg->rw_WIM;
		}

		if (w_msg->rw_isTrapSaveCounter == true)
		{
			sregister->setRegister(17, w_msg->rw_PC);
			sregister->setRegister(18, w_msg->rw_nPC);
			sregister->setPC(w_msg->rw_TBR);
			sregister->setnPC(w_msg->rw_TBR);
		}

		//sregister->dumpRegisterFile();
		//cout << "\n\n\n\n\n";

		delete w_inst;
		delete w_msg;

		pmc_instructions++;
	}

	m_exceptionStage_writebackStage_interface->setBusy(false);
}

std::string* writebackStage::getStatistics()
{
	return 0;
}

core* writebackStage::getContainingCore()
{
	return m_containingCore;
}

void writebackStage::setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface)
{
	m_exceptionStage_writebackStage_interface = x_exceptionStage_writebackStage_interface;
}

counterType writebackStage::getNumberOfInstructions()
{
	return pmc_instructions;
}

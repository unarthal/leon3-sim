#include "architecture/processor/core/decodeStage.h"
#include "architecture/processor/core/core.h"
#include <iostream>
#include "architecture/processor/core/fetchStage.h"
#include "architecture/system.h"
#include "generic/utility.h"

decodeStage::decodeStage(core* x_containingCore) : element()
{
	m_containingCore = x_containingCore;
	m_fetchStage_decodeStage_interface = 0;
	m_decodeStage_registerAccessStage_interface = 0;
	m_registerAccessStage = m_containingCore->getRegisterAccessStage();
	m_hasTrapOccurred = false;
}

decodeStage::~decodeStage()
{

}

void decodeStage::simulateOneCycle()
{
	if(m_fetchStage_decodeStage_interface->doesElementHaveAnyPendingMessage(this) == true && m_hasTrapOccurred == true)
	{
		fetchStagedecodeStageMessage* d_msg = (fetchStagedecodeStageMessage*) (m_fetchStage_decodeStage_interface->peekElementsPendingMessage(this)); //note we peek here. busy is set to true if it is a memory instruction. we pop when the response arrives. busy is set to false then.
		addrType d_pc = d_msg->getDPC();
		if(d_pc == m_containingCore->getExceptionStage()->getNewPCAfterTrap())
		{
			m_hasTrapOccurred = false;
		}
		else
		{
			//assuming there can be only one pending message
			m_fetchStage_decodeStage_interface->popElementsPendingMessage(this);
			delete d_msg;
			return;
		}
	}

	if(m_fetchStage_decodeStage_interface->doesElementHaveAnyPendingMessage(this) == true
			&& m_decodeStage_registerAccessStage_interface->getBusy() == false)
	{
		fetchStagedecodeStageMessage* d_msg = (fetchStagedecodeStageMessage*) (m_fetchStage_decodeStage_interface->popElementsPendingMessage(this));
		unsigned int d_inst = (unsigned int)(convertBigEndianByteArrayToLittleEndianInteger(d_msg->getDInst(), 4));
		addrType d_pc = d_msg->getDPC();
		//std::cout << dec << "[" << getClock() << "] D RECV : " << hex << d_pc << " : " << d_inst << dec << "\n";
		delete d_msg->getDInst();
		delete d_msg; //remember to delete messages and events once their work is done!

		int sign_extended_disp22, sign_extended_simm13;

		instruction* r_inst = new instruction();

		r_inst->instructionWord = d_inst;

		r_inst->op = extract(d_inst, 30, 31);
		if (r_inst->op == 1) // Format - I instruction
		{
			r_inst->disp30 = extract(d_inst, 0, 29);
		}
		else if (r_inst->op == 0) // Format - II instruction
		{
			r_inst->op2 = extract(d_inst,22,24);

			//SETHI and NOP
			if(r_inst->op2 == 4)
			{
				r_inst->rd = extract(d_inst,25,29);
				r_inst->imm22 = extract(d_inst,0,21);
				if(r_inst->rd == 0 && r_inst->imm22 == 0)
				{
					//NOP <- SETHI with rd==0 and imm22==0
					r_inst->rd = -1;
					r_inst->imm22 = -1;
				}
			}
			else
			{
				r_inst->a = extract(d_inst,29,29);
				r_inst->cond = extract(d_inst,25,28);

				unsigned int disp22 = extract(d_inst,0,21);
				sign_extended_disp22 = (disp22 & 0x3FFFFF) | ((disp22 & 0x20000) ? 0xFFC00000 : 0);
				r_inst->disp22 = sign_extended_disp22;
			}
		}
		else if (r_inst->op == 2 || r_inst->op == 3) //Format - III instruction
		{
			r_inst->rd = extract(d_inst,25,29);
			r_inst->op3 = extract(d_inst,19,24);
			r_inst->rs1 = extract(d_inst,14,18);

			r_inst->opf = extract(d_inst,5,13);
			r_inst->rs2 = extract(d_inst,0,4);

			r_inst->i = extract(d_inst,13,13);
			if (r_inst->i == 1)
			{
				unsigned int simm13 = extract(d_inst,0,12);
				sign_extended_simm13 = (simm13 & 0x1FFF) | ((simm13 & 0x1000) ? 0xFFFFE000 : 0);

				r_inst->simm13 = sign_extended_simm13;
				r_inst->imm7 = extract(d_inst,0,6);
			}
			else
			{
				r_inst->asi = extract(d_inst,5,12);
				r_inst->rs2 = extract(d_inst,0,4);
			}
		}

		decodeStageregisterAccessStageMessage* r_msg = new decodeStageregisterAccessStageMessage(this, m_registerAccessStage, r_inst, d_pc);
		m_decodeStage_registerAccessStage_interface->addPendingMessage(r_msg);
	}

	if(m_decodeStage_registerAccessStage_interface->getBusy() == true
			&& m_fetchStage_decodeStage_interface->doesElementHaveAnyPendingMessage(this) == true)
	{
		m_fetchStage_decodeStage_interface->setBusy(true);
	}
	else
	{
		m_fetchStage_decodeStage_interface->setBusy(false);
	}
}

void decodeStage::setHasTrapOccurred()
{
	m_hasTrapOccurred = true;
}

std::string* decodeStage::getStatistics()
{
	return 0;
}

core* decodeStage::getContainingCore()
{
	return m_containingCore;
}

void decodeStage::setFetchDecodeInterface(interface* x_fetchStage_decodeStage_interface)
{
	m_fetchStage_decodeStage_interface = x_fetchStage_decodeStage_interface;
}

void decodeStage::setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStageinterface)
{
	m_decodeStage_registerAccessStage_interface = x_decodeStage_registerAccessStageinterface;
}

decodeStageregisterAccessStageMessage::decodeStageregisterAccessStageMessage(element* x_producer, element* x_consumer, instruction* x_r_inst, unsigned int x_r_pc) : message(x_producer, x_consumer)
{
	m_r_inst = x_r_inst;
	m_r_pc = x_r_pc;
}

decodeStageregisterAccessStageMessage::~decodeStageregisterAccessStageMessage()
{

}

instruction* decodeStageregisterAccessStageMessage::getRInst()
{
	return m_r_inst;
}

unsigned int decodeStageregisterAccessStageMessage::getRPC()
{
	return m_r_pc;
}

#include "architecture/processor/core/registerAccessStage.h"
#include "architecture/processor/core/core.h"
#include <iostream>
#include "architecture/processor/core/decodeStage.h"
#include "architecture/system.h"
#include "generic/utility.h"
#include "architecture/processor/core/registers/registerfile.h"

registerAccessStage::registerAccessStage(core* x_containingCore) : element()
{
	m_containingCore = x_containingCore;
	m_decodeStage_registerAccessStage_interface = 0;
	m_registerAccessStage_executeStage_interface = 0;
	m_executeStage = m_containingCore->getExecuteStage();
	m_hasTrapOccurred = false;
}

registerAccessStage::~registerAccessStage()
{

}

void registerAccessStage::simulateOneCycle()
{
	if(m_decodeStage_registerAccessStage_interface->doesElementHaveAnyPendingMessage(this) == true && m_hasTrapOccurred == true)
	{
		decodeStageregisterAccessStageMessage* r_msg = (decodeStageregisterAccessStageMessage*) (m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(this)); //note we peek here. busy is set to true if it is a memory instruction. we pop when the response arrives. busy is set to false then.
		addrType r_pc = r_msg->getRPC();
		if(r_pc == m_containingCore->getExceptionStage()->getNewPCAfterTrap())
		{
			m_hasTrapOccurred = false;
		}
		else
		{
			//assuming there can be only one pending message
			m_decodeStage_registerAccessStage_interface->popElementsPendingMessage(this);
			delete r_msg;
			return;
		}
	}

	if(m_decodeStage_registerAccessStage_interface->doesElementHaveAnyPendingMessage(this) == true
			&& m_registerAccessStage_executeStage_interface->getBusy() == false)
	{
		decodeStageregisterAccessStageMessage* r_msg = (decodeStageregisterAccessStageMessage*) (m_decodeStage_registerAccessStage_interface->popElementsPendingMessage(this));
		instruction* r_inst = r_msg->getRInst();
		addrType r_pc = r_msg->getRPC();
		//std::cout << dec << "[" << getClock() << "] RA RECV : " << hex << r_pc << " : " << r_inst->instructionWord << dec << "\n";
		delete r_msg; //remember to delete messages and events once their work is done!

		//data hazards have already been checked. if the control reaches here, there are no data hazards.

		registerAccessStageexecuteStageMessage* e_msg = new registerAccessStageexecuteStageMessage(this, m_executeStage, r_inst, r_pc);

		e_msg->l_PSR = m_containingCore->getRegisterFile()->SRegisters->psr->getPSR();
		e_msg->l_FSR = m_containingCore->getRegisterFile()->SRegisters->fsr->getFSR();
		e_msg->l_ASR = m_containingCore->getRegisterFile()->getASRRegister(r_inst->rs1); //hardcoded to rs1 as in ReadStateRegisterInstructions
		e_msg->l_TBR = m_containingCore->getRegisterFile()->SRegisters->tbr->getTbr();


		e_msg->l_PSR_EF = m_containingCore->getRegisterFile()->SRegisters->psr->getEf();
		e_msg->l_PSR_ET = m_containingCore->getRegisterFile()->SRegisters->psr->getEt();
		e_msg->l_PSR_S = m_containingCore->getRegisterFile()->SRegisters->psr->getS();
		e_msg->l_PSR_PS = m_containingCore->getRegisterFile()->SRegisters->psr->getS();

		e_msg->l_FSR_fcc = m_containingCore->getRegisterFile()->SRegisters->fsr->getFcc();
		e_msg->l_FSR_RD = m_containingCore->getRegisterFile()->SRegisters->fsr->getRd();

		e_msg->l_WIM = m_containingCore->getRegisterFile()->SRegisters->wim;
		e_msg->l_CWP = m_containingCore->getRegisterFile()->SRegisters->psr->getCwp();
		e_msg->l_nWP = m_containingCore->getRegisterFile()->SRegisters->totalRegisterWindows;
		//e_msg->l_PC = m_containingCore->getRegisterFile()->getPC();
		//e_msg->l_nPC = m_containingCore->getRegisterFile()->getnPC();
		e_msg->l_PC = r_pc;
		e_msg->l_nPC = r_pc+4;


		///processor state register icc codes
		e_msg->l_PSR_icc_N = m_containingCore->getRegisterFile()->SRegisters->psr->getN();
		e_msg->l_PSR_icc_Z = m_containingCore->getRegisterFile()->SRegisters->psr->getZ();
		e_msg->l_PSR_icc_V = m_containingCore->getRegisterFile()->SRegisters->psr->getV();
		e_msg->l_PSR_icc_C = m_containingCore->getRegisterFile()->SRegisters->psr->getC();

		///Multiply/Divide Register (MSB 32 bits of product/dividend)
		e_msg->l_Y = m_containingCore->getRegisterFile()->getY();

		if (r_inst->op == 1 ){ //f1
			//No int/flt regload required
			//only pc ang reg15 stp are changed
		} else if (r_inst->op == 0){ //f2 SETHI/Branches
			if (r_inst->op2 == 4){
				// SETHI (reg write to rd only)
				//No int/flt regload required
			} else if (r_inst->op2 == 2 ){
				//Integer Branch
				//No int/flt regload required
				// only pc,npc, psr
			} else if (r_inst->op2 == 6 ){
				//Floatingpoint Branch
				//No int/flt regload required
				// only pc,npc, psr
			}

		} else if (r_inst->op == 2){ //f3 Arith/Logic
			if (r_inst->op3 == 52 || r_inst->op3 == 53 ){ // FP Instructions

				e_msg->l_regFs1_1 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs1);
				e_msg->l_regFs1_2 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs1+1);
				e_msg->l_regFs1_3 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs1+2);
				e_msg->l_regFs1_4 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs1+3);

				e_msg->l_regFs2_1 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs2);
				e_msg->l_regFs2_2 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs2+1);
				e_msg->l_regFs2_3 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs2+2);
				e_msg->l_regFs2_4 = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rs2+3);

				e_msg->l_regRD = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rd);
				e_msg->l_regNextRD = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rd+1);

			} else { // Integer Instructions (to CHECK )
				if(r_inst->op3 == 58){ //Trap on ICC (imm7 case)
					e_msg->l_regRS1 = m_containingCore->getRegisterFile()->getRegister(r_inst->rs1);
					e_msg->l_reg_or_imm = r_inst->i ? r_inst->imm7 : m_containingCore->getRegisterFile()->getRegister(r_inst->rs2);
					// icc's NZVC common above
				} else {
					e_msg->l_regRS1 = m_containingCore->getRegisterFile()->getRegister(r_inst->rs1);
					e_msg->l_reg_or_imm = r_inst->i ? r_inst->simm13 : m_containingCore->getRegisterFile()->getRegister(r_inst->rs2);
					if(r_inst->i==0){
					e_msg->l_regRS2 = m_containingCore->getRegisterFile()->getRegister(r_inst->rs2);
					}
					e_msg->l_regRD = m_containingCore->getRegisterFile()->getRegister(r_inst->rd);
					e_msg->l_regNextRD = m_containingCore->getRegisterFile()->getRegister(r_inst->rd+1); // to check
					if(r_inst->op3 == 57/*RETT*/)
					{
						e_msg->getEInst()->rd = -1;
						e_msg->l_regRD = -1;
						e_msg->l_regNextRD = -1;
					}
				}



			}
		// Co-Processor Instructions Not Implemented

		} else if (r_inst->op == 3){ //f3 Memory
			e_msg->l_regRS1 = m_containingCore->getRegisterFile()->getRegister(r_inst->rs1);
			e_msg->l_reg_or_imm = r_inst->i ? r_inst->simm13 : m_containingCore->getRegisterFile()->getRegister(r_inst->rs2);
			if(r_inst->i==0){
				e_msg->l_regRS2 = m_containingCore->getRegisterFile()->getRegister(r_inst->rs2);
			}

			if(r_inst->op3==36 || r_inst->op3==39 || r_inst->op3==37 || r_inst->op3==38){ //// Store float point
				e_msg->l_regRD = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rd);
				e_msg->l_regNextRD = m_containingCore->getRegisterFile()->getFloatingRegister(r_inst->rd+1);
			} else { //// store integer or load int/flt
				e_msg->l_regRD = m_containingCore->getRegisterFile()->getRegister(r_inst->rd);
				e_msg->l_regNextRD = m_containingCore->getRegisterFile()->getRegister(r_inst->rd+1);
			}
		}

		////FOR XC
		e_msg->xc_psr = m_containingCore->getRegisterFile()->SRegisters->psr->getPSR();
		e_msg->xc_tbr = m_containingCore->getRegisterFile()->SRegisters->tbr->getTbr();
		e_msg->xc_PC = m_containingCore->getRegisterFile()->getPC();
		e_msg->xc_nPC = m_containingCore->getRegisterFile()->getnPC();
		e_msg->xc_tbr = m_containingCore->getRegisterFile()->SRegisters->tbr->getTbr();
		e_msg->xc_nWP = m_containingCore->getRegisterFile()->SRegisters->totalRegisterWindows;

		m_registerAccessStage_executeStage_interface->addPendingMessage(e_msg);
	}

	if(m_registerAccessStage_executeStage_interface->getBusy() == true
			&& m_decodeStage_registerAccessStage_interface->doesElementHaveAnyPendingMessage(this) == true)
	{
		m_decodeStage_registerAccessStage_interface->setBusy(true);
	}
	else
	{
		m_decodeStage_registerAccessStage_interface->setBusy(false);
	}
}

void registerAccessStage::setHasTrapOccurred()
{
	m_hasTrapOccurred = true;
}

std::string* registerAccessStage::getStatistics()
{
	return 0;
}

core* registerAccessStage::getContainingCore()
{
	return m_containingCore;
}

void registerAccessStage::setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStage_interface)
{
	m_decodeStage_registerAccessStage_interface = x_decodeStage_registerAccessStage_interface;
}

void registerAccessStage::setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface)
{
	m_registerAccessStage_executeStage_interface = x_registerAccessStage_executeStage_interface;
}

registerAccessStageexecuteStageMessage::registerAccessStageexecuteStageMessage(element* x_producer, element* x_consumer, instruction* x_e_inst, unsigned int x_e_pc) : message(x_producer, x_consumer)
{
	m_e_inst = x_e_inst;
	m_e_pc = x_e_pc;

	l_regRS1 = 0;
	l_reg_or_imm = 0;
	l_regRS2 = 0;
	l_regRD = 0; //common
	l_regNextRD = 0; //common

	////Floating Point
	l_regFs1_1 = 0;
	l_regFs1_2 = 0;
	l_regFs1_3 = 0;
	l_regFs1_4 = 0;
	l_regFs2_1 = 0;
	l_regFs2_2 = 0;
	l_regFs2_3 = 0;
	l_regFs2_4 = 0;

	l_PSR_EF = 0;
	l_PSR_ET = 0;
	l_PSR_S = 0;
	l_PSR_PS = 0;

	l_WIM = 0;
	l_PSR = 0;
	l_FSR = 0;
	l_ASR = 0;
	l_TBR = 0;
	l_Y = 0;

	l_PSR_icc_N = 0;
	l_PSR_icc_Z = 0;
	l_PSR_icc_V = 0;
	l_PSR_icc_C = 0;
	l_FSR_RD = 0;

	l_CWP = 0;
	l_nWP = 0;

	l_FSR_fcc = 0;
	l_PC = 0;
	l_nPC = 0;
	//  int l_ASRreg;

	//for the XC stage
	xc_isError = false;
	xc_psr = 0;
	xc_tbr = 0;
	xc_nWP = 0;
	xc_PC = 0;
	xc_nPC = 0;
}

registerAccessStageexecuteStageMessage::~registerAccessStageexecuteStageMessage()
{

}

instruction* registerAccessStageexecuteStageMessage::getEInst()
{
	return m_e_inst;
}

unsigned int registerAccessStageexecuteStageMessage::getEPC()
{
	return m_e_pc;
}

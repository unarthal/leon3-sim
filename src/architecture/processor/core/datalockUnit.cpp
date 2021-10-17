#include "architecture/processor/core/datalockUnit.h"
#include "architecture/processor/core/core.h"
#include "architecture/processor/core/decodeStage.h"
#include "architecture/processor/core/registerAccessStage.h"
#include "architecture/processor/core/executeStage.h"
#include "architecture/processor/core/memoryStage.h"
#include "architecture/processor/core/exceptionStage.h"

datalockUnit::datalockUnit(core* x_containingCore)
{
	m_containingCore = x_containingCore;
}

datalockUnit::~datalockUnit()
{

}

bool datalockUnit::isRAWHazard(int x_sourceReg)
{
	//assuming that each latch should only contain one instruction
	//TODO instructions with double precision floating point results
	if(x_sourceReg == -1)
		return false;

	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0)
	{
		instruction* inst = ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst();
		if(inst->rd == x_sourceReg
				|| (/*LDD*/inst->op == 3 && inst->op3 == 3 && inst->rd+1 == x_sourceReg))
		{
			return true;
		}
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0)
	{
		instruction* inst = ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst();
		if(inst->rd == x_sourceReg
				|| (/*LDD*/inst->op == 3 && inst->op3 == 3 && inst->rd+1 == x_sourceReg))
		{
			return true;
		}
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0)
	{
		instruction* inst = ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst();
		if(inst->rd == x_sourceReg
				|| (/*LDD*/inst->op == 3 && inst->op3 == 3 && inst->rd+1 == x_sourceReg))
		{
			return true;
		}
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0)
	{
		instruction* inst = ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst();
		if(inst->rd == x_sourceReg
				|| (/*LDD*/inst->op == 3 && inst->op3 == 3 && inst->rd+1 == x_sourceReg))
		{
			return true;
		}
	}

	return false;
}

bool datalockUnit::anyWriterToConditionCodes()
{
	//assuming that each latch should only contain one instruction
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0
			&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isConditionCodeWriter())
	{
		return true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0
			&& ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isConditionCodeWriter())
	{
		return true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0
			&& ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isConditionCodeWriter())
	{
		return true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0
			&& ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isConditionCodeWriter())
	{
		return true;
	}

	return false;
}

bool datalockUnit::anyCWPModifyingInstruction()
{
	//assuming that each latch should only contain one instruction
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0
			&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isCWPModifyingInstruction())
	{
		return true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0
			&& ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isCWPModifyingInstruction())
	{
		return true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0
			&& ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isCWPModifyingInstruction())
	{
		return true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0
			&& ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isCWPModifyingInstruction())
	{
		return true;
	}

	return false;
}

bool datalockUnit::anyDataHazard()
{
	if(m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()) == 0)
		return false;

	if(anyCWPModifyingInstruction())
	{
		return true;
	}

	instruction* r_inst = ((decodeStageregisterAccessStageMessage*)m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()))->getRInst();

	if((r_inst->rs1 != -1 && isRAWHazard(r_inst->rs1) == true)
			|| (r_inst->i == 0 && r_inst->rs2 != -1 && isRAWHazard(r_inst->rs2) == true)
			|| (/*stores use rd*/ r_inst->op == 3 && ((r_inst->op3 >= 4 && r_inst->op3 <= 7) || (r_inst->op3 >= 36 && r_inst->op3 <= 39)) && isRAWHazard(r_inst->rd) == true)
			|| (/*conditional branches need to read condition codes*/r_inst->isConditionalControlTransferInstruction() == true && anyWriterToConditionCodes() == true))
	{
		return true;
	}

	return false;
}

core* datalockUnit::getContainingCore()
{
	return m_containingCore;
}

void datalockUnit::setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStage_interface)
{
	m_decodeStage_registerAccessStage_interface = x_decodeStage_registerAccessStage_interface;
}

void datalockUnit::setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface)
{
	m_registerAccessStage_executeStage_interface = x_registerAccessStage_executeStage_interface;
}

void datalockUnit::setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface)
{
	m_executeStage_memoryStage_interface = x_executeStage_memoryStage_interface;
}

void datalockUnit::setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface)
{
	m_memoryStage_exceptionStage_interface = x_memoryStage_exceptionStage_interface;
}

void datalockUnit::setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface)
{
	m_exceptionStage_writebackStage_interface = x_exceptionStage_writebackStage_interface;
}


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

/*
 * RAW hazards in non-general purpose registers
 *
 * (1) PSR: icc
 * writers: any XXcc instruction (e.g., ANDcc), WRPSR
 * readers: Bicc, Ticc
 *
 * (2) PSR: CWP
 * writers: SAVE, RESTORE, RETT, exception stage (causes pipeline flush, so no special handling required)
 * readers: any subsequent instruction
 *
 * (3) Y
 * writers: SMUL, SMULcc, UMUL, UMULcc, MULScc, WRY
 * readers: MULScc, SDIV, SDIVcc, UDIV, UDIVcc, RDY
 *
 * (4) ASR
 * writers: WRASR
 * readers: RDASR
 *
 * (5) WIM
 * writers: WRWIM
 * readers: RDWIM
 *
 * (6) TBR
 * writers: WRTBR, exception stage (causes pipeline flush, so no special handling required)
 * readers: exception stage
 */

bool datalockUnit::isRAWHazard_GeneralPurpose(int x_sourceReg)
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
			&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isICCWritingInstruction())
	{
		return true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0
			&& ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isICCWritingInstruction())
	{
		return true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0
			&& ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isICCWritingInstruction())
	{
		return true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0
			&& ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isICCWritingInstruction())
	{
		return true;
	}

	return false;
}

bool datalockUnit::anyCWPWritingInstruction()
{
	//assuming that each latch should only contain one instruction
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0
			&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isCWPWritingInstruction())
	{
		return true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0
			&& ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isCWPWritingInstruction())
	{
		return true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0
			&& ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isCWPWritingInstruction())
	{
		return true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0
			&& ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isCWPWritingInstruction())
	{
		return true;
	}

	return false;
}

bool datalockUnit::anyYWritingInstruction()
{
	//assuming that each latch should only contain one instruction
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0
			&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isYWritingInstruction())
	{
		return true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0
			&& ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isYWritingInstruction())
	{
		return true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0
			&& ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isYWritingInstruction())
	{
		return true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0
			&& ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isYWritingInstruction())
	{
		return true;
	}

	return false;
}

bool datalockUnit::anyASRWritingInstruction()
{
	//assuming that each latch should only contain one instruction
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0
			&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isASRWritingInstruction())
	{
		return true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0
			&& ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isASRWritingInstruction())
	{
		return true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0
			&& ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isASRWritingInstruction())
	{
		return true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0
			&& ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isASRWritingInstruction())
	{
		return true;
	}

	return false;
}

bool datalockUnit::anyWIMWritingInstruction()
{
	//assuming that each latch should only contain one instruction
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0
			&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isWIMWritingInstruction())
	{
		return true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0
			&& ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isWIMWritingInstruction())
	{
		return true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0
			&& ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isWIMWritingInstruction())
	{
		return true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0
			&& ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isWIMWritingInstruction())
	{
		return true;
	}

	return false;
}

bool datalockUnit::anyDataHazard()
{
	if(m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()) == 0)
		return false;

	if(anyCWPWritingInstruction())
	{
		return true;
	}

	instruction* r_inst = ((decodeStageregisterAccessStageMessage*)m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()))->getRInst();

	if((r_inst->rs1 != -1 && isRAWHazard_GeneralPurpose(r_inst->rs1) == true)
			|| (r_inst->i == 0 && r_inst->rs2 != -1 && isRAWHazard_GeneralPurpose(r_inst->rs2) == true)
			|| (/*stores use rd*/ r_inst->op == 3 && ((r_inst->op3 >= 4 && r_inst->op3 <= 7) || (r_inst->op3 >= 36 && r_inst->op3 <= 39)) && isRAWHazard_GeneralPurpose(r_inst->rd) == true)
			|| (/*conditional branches need to read condition codes*/r_inst->isConditionalControlTransferInstruction() == true && anyWriterToConditionCodes() == true)
			|| (/*Y register dependency*/r_inst->isYReadingInstruction() == true && anyYWritingInstruction() == true)
			|| (/*ASR register dependency*/r_inst->isASRReadingInstruction() == true && anyASRWritingInstruction() == true /*TODO differentiate between the various ASRs*/)
			|| (/*WIM register dependency*/r_inst->isWIMReadingInstruction() == true && anyWIMWritingInstruction() == true))
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


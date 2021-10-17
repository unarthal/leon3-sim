#include "architecture/processor/core/controllockUnit.h"
#include "architecture/processor/core/core.h"
#include "architecture/processor/core/fetchStage.h"
#include "architecture/processor/core/decodeStage.h"
#include "architecture/processor/core/registerAccessStage.h"
#include "architecture/processor/core/executeStage.h"
#include "architecture/processor/core/memoryStage.h"
#include "architecture/processor/core/exceptionStage.h"

controllockUnit::controllockUnit(core* x_containingCore)
{
	m_containingCore = x_containingCore;
}

controllockUnit::~controllockUnit()
{

}

bool controllockUnit::anyControlHazard()
{
	//assuming that each latch should only contain one instruction
	//sparcv8 prescribes one delay slot
	//so the immediate preceding instruction need not be considered
	//if any other instruction is a control instruction, then it is a control hazard

	bool pastFirstPrecedingInstruction = false;
	if(m_fetchStage_decodeStage_interface->peekElementsPendingMessage(m_containingCore->getDecodeStage()) != 0)
	{
		pastFirstPrecedingInstruction = true;
	}
	if(m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()) != 0)
	{
		if(pastFirstPrecedingInstruction && ((decodeStageregisterAccessStageMessage*)m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()))->getRInst()->isControlTransferInstruction())
		{
			return true;
		}
		pastFirstPrecedingInstruction = true;
	}
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0)
	{
		if(pastFirstPrecedingInstruction && ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEInst()->isControlTransferInstruction())
		{
			return true;
		}
		pastFirstPrecedingInstruction = true;
	}
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()) != 0)
	{
		if(pastFirstPrecedingInstruction && ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_containingCore->getMemoryStage()))->getMInst()->isControlTransferInstruction())
		{
			return true;
		}
		pastFirstPrecedingInstruction = true;
	}
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()) != 0)
	{
		if(pastFirstPrecedingInstruction && ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_containingCore->getExceptionStage()))->getXInst()->isControlTransferInstruction())
		{
			return true;
		}
		pastFirstPrecedingInstruction = true;
	}
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()) != 0)
	{
		if(pastFirstPrecedingInstruction && ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_containingCore->getWritebackStage()))->getWInst()->isControlTransferInstruction())
		{
			return true;
		}
		pastFirstPrecedingInstruction = true;
	}

	return false;
}

void controllockUnit::performAnnulment(addrType x_PC_to_be_annuled)
{
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()) != 0
				&& ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_containingCore->getExecuteStage()))->getEPC() == x_PC_to_be_annuled)
	{
		m_registerAccessStage_executeStage_interface->popElementsPendingMessage(m_containingCore->getExecuteStage());
	}

	if(m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()) != 0
				&& ((decodeStageregisterAccessStageMessage*)m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_containingCore->getRegisterAccessStage()))->getRPC() == x_PC_to_be_annuled)
	{
		m_decodeStage_registerAccessStage_interface->popElementsPendingMessage(m_containingCore->getRegisterAccessStage());
	}

	if(m_fetchStage_decodeStage_interface->peekElementsPendingMessage(m_containingCore->getDecodeStage()) != 0
				&& ((fetchStagedecodeStageMessage*)m_fetchStage_decodeStage_interface->peekElementsPendingMessage(m_containingCore->getDecodeStage()))->getDPC() == x_PC_to_be_annuled)
	{
		m_fetchStage_decodeStage_interface->popElementsPendingMessage(m_containingCore->getDecodeStage());
	}
}

core* controllockUnit::getContainingCore()
{
	return m_containingCore;
}

void controllockUnit::setFetchDecodeInterface(interface* x_fetchStage_decodeStage_interface)
{
	m_fetchStage_decodeStage_interface = x_fetchStage_decodeStage_interface;
}

void controllockUnit::setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStage_interface)
{
	m_decodeStage_registerAccessStage_interface = x_decodeStage_registerAccessStage_interface;
}

void controllockUnit::setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface)
{
	m_registerAccessStage_executeStage_interface = x_registerAccessStage_executeStage_interface;
}

void controllockUnit::setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface)
{
	m_executeStage_memoryStage_interface = x_executeStage_memoryStage_interface;
}

void controllockUnit::setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface)
{
	m_memoryStage_exceptionStage_interface = x_memoryStage_exceptionStage_interface;
}

void controllockUnit::setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface)
{
	m_exceptionStage_writebackStage_interface = x_exceptionStage_writebackStage_interface;
}


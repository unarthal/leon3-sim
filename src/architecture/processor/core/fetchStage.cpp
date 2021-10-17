#include "architecture/processor/core/fetchStage.h"
#include <strings.h>
#include "generic/utility.h"
#include "architecture/memory/memory.h"
#include "architecture/processor/core/core.h"
#include "architecture/interface.h"
#include "architecture/processor/processor.h"
#include "architecture/memory/simple_memory.h" //TODO fetchStage should be oblivious of the type of memory being employed
#include "architecture/system.h"
#include <iostream>
#include "architecture/processor/core/registers/registerfile.h"
#include "elf/elf.h"
#include "architecture/processor/core/decodeStage.h"
#include "architecture/processor/core/controllockUnit.h"

fetchStage::fetchStage(core* x_containingCore) : element()
{
	m_containingCore = x_containingCore;
	m_fetchStage_decodeStage_interface = 0;
	m_decodeStage = m_containingCore->getDecodeStage();
	m_hasTrapOccurred = false;
}

fetchStage::~fetchStage()
{

}

extern int verbose;
void fetchStage::simulateOneCycle()
{
	interface* processor_memory_interface = m_containingCore->getContainingProcessor()->getProcessorMemoryInterface();
	interface* memory_processor_interface = m_containingCore->getContainingProcessor()->getMemoryProcessorInterface();

	if(memory_processor_interface->doesElementHaveAnyPendingMessage(this) == true && m_hasTrapOccurred == true)
	{
		memorymessage* m_msg = (memorymessage*) (memory_processor_interface->peekElementsPendingMessage(this)); //note we peek here. busy is set to true if it is a memory instruction. we pop when the response arrives. busy is set to false then.
		addrType m_pc = m_msg->getAddress();
		if(m_pc == m_containingCore->getExceptionStage()->getNewPCAfterTrap())
		{
			m_hasTrapOccurred = false;
		}
		else
		{
			//assuming there can be only one pending message
			memory_processor_interface->popElementsPendingMessage(this);
			delete m_msg;
			return;
		}
	}

	if(memory_processor_interface->doesElementHaveAnyPendingMessage(this) == true
			&& m_fetchStage_decodeStage_interface->getBusy() == false)
	{
		memorymessage* m_msg = (memorymessage*) (memory_processor_interface->popElementsPendingMessage(this));
		//std::cout << dec << "[" << getClock() << "] IF RESP : " << m_msg->getMemoryMessageType() << " : " << hex << m_msg->getAddress() << dec << "\n";

		fetchStagedecodeStageMessage* d_msg = new fetchStagedecodeStageMessage(this, m_decodeStage, m_msg->getValue(), m_msg->getAddress());
		m_fetchStage_decodeStage_interface->addPendingMessage(d_msg);

		delete m_msg; //remember to delete messages and events once their work is done!
	}

//	if(m_hasTrapOccurred == true && m_containingCore->getRegisterFile()->getnPC() != m_containingCore->getExceptionStage()->getNewPCAfterTrap())
//	{
//		//instruction that caused the trap hasn't left the pipeline
//		return;
//	}

	if(processor_memory_interface->getBusy() == false
			&& m_fetchStage_decodeStage_interface->getBusy() == false
			&& m_containingCore->getControlLockUnit()->anyControlHazard() == false)
	{
		addrType PC = m_containingCore->getRegisterFile()->getPC();
		if(PC == getMainEndAddress())
		{
			setSimulationDone(true);
			if(verbose != 0)
			{
				m_containingCore->getRegisterFile()->dumpRegisterFile();
				m_containingCore->getContainingProcessor()->getAttachedMemory()->dumpMemory(0xfffffed0, 0xfffffffe);
			}
		}

		if (is_mem_address_not_aligned(PC, WORD_ALIGN))
		{
			cout << ("memory address not word aligned") << endl;
			//returnValue = mem_address_not_aligned;
		}
		memorymessage* msg = new memorymessage(this, m_containingCore->getContainingProcessor()->getAttachedMemory(), Read, PC, 4, 0); //TODO depending upon the level of implementation detail, the message contents would vary. how to elegantly code this?
		processor_memory_interface->addPendingMessage(msg);

		addrType nPC = m_containingCore->getRegisterFile()->getnPC();
		m_containingCore->getRegisterFile()->setPC(nPC);
		m_containingCore->getRegisterFile()->setnPC(nPC+4);
		//std::cout << dec << "[" << getClock() << "] IF ISSUE: " << msg->getMemoryMessageType() << " : " << hex << msg->getAddress() << dec << "\n";
	}
}

void fetchStage::setHasTrapOccurred()
{
	m_hasTrapOccurred = true;
}

std::string* fetchStage::getStatistics()
{
	return 0;
}

core* fetchStage::getContainingCore()
{
	return m_containingCore;
}

void fetchStage::setFetchDecodeInterface(interface* x_fetchStage_decodeStage_interface)
{
	m_fetchStage_decodeStage_interface = x_fetchStage_decodeStage_interface;
}

fetchStagedecodeStageMessage::fetchStagedecodeStageMessage(element* x_producer, element* x_consumer, char* x_d_inst, addrType x_d_pc) : message(x_producer, x_consumer)
{
	m_d_inst = x_d_inst;
	m_d_pc = x_d_pc;
}

fetchStagedecodeStageMessage::~fetchStagedecodeStageMessage()
{

}

char* fetchStagedecodeStageMessage::getDInst()
{
	return m_d_inst;
}

addrType fetchStagedecodeStageMessage::getDPC()
{
	return m_d_pc;
}

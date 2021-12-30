#include "architecture/processor/core/core.h"
#include "architecture/processor/processor.h"
#include "elf/elf.h"
#include "architecture/processor/core/fetchStage.h"
#include "architecture/processor/core/decodeStage.h"
#include "architecture/processor/core/registerAccessStage.h"
#include "architecture/processor/core/executeStage.h"
#include "architecture/processor/core/memoryStage.h"
#include "architecture/processor/core/exceptionStage.h"
#include "architecture/processor/core/writebackStage.h"
#include "architecture/processor/core/registers/registerfile.h"
#include "architecture/processor/core/datalockUnit.h"
#include "architecture/processor/core/controllockUnit.h"
#include "architecture/interface.h"
#include <iostream>
#include "architecture/system.h"
#include "architecture/processor/cache/cache.h"
#include "config/config.h"
#include <string>

core::core(processor* x_containingProcessor, int x_numberOfRegisterWindows) : element()
{
	m_containingProcessor = x_containingProcessor;

	m_sregister = new registerfile();
	m_sregister->initializeRegisters(x_numberOfRegisterWindows, getMainStartAddress());

	m_writebackStage = new writebackStage(this);
	m_exceptionStage = new exceptionStage(this);
	m_memoryStage = new memoryStage(this);
	m_executeStage = new executeStage(this);
	m_registerAccessStage = new registerAccessStage(this);
	m_decodeStage = new decodeStage(this);
	m_fetchStage = new fetchStage(this);

	m_datalockUnit = new datalockUnit(this);
	m_controllockUnit = new controllockUnit(this);

// I-cache and D-cache
	m_icache = new cache(this);
	m_icache->setCacheSize(xmlReader_generalInt("src/config/config.xml","<iCacheSize>","</iCacheSize>"));
	m_icache->setLineSize(xmlReader_generalInt("src/config/config.xml","<iLineSize>","</iLineSize>"));
	m_icache->setReplacementPolicy(xmlReader_generalInt("src/config/config.xml","<iReplacementPolicy>","</iReplacementPolicy>"));
	m_icache->setLatency(xmlReader_generalInt("src/config/config.xml","<iLatency>","</iLatency>"));
	m_icache->setSetAssociativity(xmlReader_generalInt("src/config/config.xml","<iSetAssociativity>","</iSetAssociativity>"));
	m_icache->initialiseCache();
	
	m_dcache = new cache(this);
	m_dcache->setCacheSize(xmlReader_generalInt("src/config/config.xml","<dCacheSize>","</dCacheSize>"));
	m_dcache->setLineSize(xmlReader_generalInt("src/config/config.xml","<dLineSize>","</dLineSize>"));
	m_dcache->setReplacementPolicy(xmlReader_generalInt("src/config/config.xml","<dReplacementPolicy>","</dReplacementPolicy>"));
	m_dcache->setLatency(xmlReader_generalInt("src/config/config.xml","<dLatency>","</dLatency>"));
	m_dcache->setSetAssociativity(xmlReader_generalInt("src/config/config.xml","<dSetAssociativity>","</dSetAssociativity>"));
	m_dcache->initialiseCache();


// I-Cache to fetchStage Interface
	m_icache_fetchStage_interface = new interface();
	m_icache->setCacheUpperlevelInterface(m_icache_fetchStage_interface);
	m_fetchStage->setIcacheFetchStageInterface(m_icache_fetchStage_interface);

// Fetch Stage to I-Cache Interface
	m_fetchStage_icache_interface = new interface();
	m_icache->setUpperlevelCacheInterface(m_fetchStage_icache_interface);
	m_fetchStage->setFetchStageIcacheInterface(m_fetchStage_icache_interface);

// Set I-cache lower-level interface as processor-memory-interface
	m_icache->setCacheLowerlevelInterface(m_containingProcessor->getProcessorMemoryInterface());
// Set lower-level I-cache interface as memory-processor-interface
	m_icache->setLowerlevelCacheInterface(m_containingProcessor->getMemoryProcessorInterface());
	
	m_icache->setUpperLevelElement(m_fetchStage);
	m_icache->setLowerlevelElement(m_containingProcessor->getAttachedMemory());

// D-Cache to memoryStage Interface
	m_dcache_memoryStage_interface = new interface();
	m_dcache->setCacheUpperlevelInterface(m_dcache_memoryStage_interface);
	m_memoryStage->setDcacheMemoryStageInterface(m_dcache_memoryStage_interface);

// Memory stage to D-cache interface
	m_memoryStage_dcache_interface = new interface();
	m_dcache->setUpperlevelCacheInterface(m_memoryStage_dcache_interface);
	m_memoryStage->setMemoryStageDcacheInterface(m_memoryStage_dcache_interface);

// Set D-cache lower-level interface as processor-memory-interface
	m_dcache->setCacheLowerlevelInterface(m_containingProcessor->getProcessorMemoryInterface());
// Set lower-level D-cache interface as memory-processor-interface
	m_dcache->setLowerlevelCacheInterface(m_containingProcessor->getMemoryProcessorInterface());

	m_dcache->setUpperLevelElement(m_memoryStage);
	m_dcache->setLowerlevelElement(m_containingProcessor->getAttachedMemory());
	
	//set up interfaces
	m_fetchStage_decodeStage_interface = new interface();
	m_fetchStage->setFetchDecodeInterface(m_fetchStage_decodeStage_interface);
	m_decodeStage->setFetchDecodeInterface(m_fetchStage_decodeStage_interface);
	m_decodeStage_registerAccessStage_interface = new interface();
	m_decodeStage->setDecodeRegisterAccessInterface(m_decodeStage_registerAccessStage_interface);
	m_registerAccessStage->setDecodeRegisterAccessInterface(m_decodeStage_registerAccessStage_interface);
	m_registerAccessStage_executeStage_interface = new interface();
	m_registerAccessStage->setRegisterAccessExecuteInterface(m_registerAccessStage_executeStage_interface);
	m_executeStage->setRegisterAccessExecuteInterface(m_registerAccessStage_executeStage_interface);
	m_executeStage_memoryStage_interface = new interface();
	m_executeStage->setExecuteMemoryInterface(m_executeStage_memoryStage_interface);
	m_memoryStage->setExecuteMemoryInterface(m_executeStage_memoryStage_interface);
	m_memoryStage_exceptionStage_interface = new interface();
	m_memoryStage->setMemoryExceptionInterface(m_memoryStage_exceptionStage_interface);
	m_exceptionStage->setMemoryExceptionInterface(m_memoryStage_exceptionStage_interface);
	m_exceptionStage_writebackStage_interface = new interface();
	m_exceptionStage->setExceptionWritebackInterface(m_exceptionStage_writebackStage_interface);
	m_writebackStage->setExceptionWritebackInterface(m_exceptionStage_writebackStage_interface);

	m_datalockUnit->setDecodeRegisterAccessInterface(m_decodeStage_registerAccessStage_interface);
	m_datalockUnit->setRegisterAccessExecuteInterface(m_registerAccessStage_executeStage_interface);
	m_datalockUnit->setExecuteMemoryInterface(m_executeStage_memoryStage_interface);
	m_datalockUnit->setMemoryExceptionInterface(m_memoryStage_exceptionStage_interface);
	m_datalockUnit->setExceptionWritebackInterface(m_exceptionStage_writebackStage_interface);
	m_controllockUnit->setFetchDecodeInterface(m_fetchStage_decodeStage_interface);
	m_controllockUnit->setDecodeRegisterAccessInterface(m_decodeStage_registerAccessStage_interface);
	m_controllockUnit->setRegisterAccessExecuteInterface(m_registerAccessStage_executeStage_interface);
	m_controllockUnit->setExecuteMemoryInterface(m_executeStage_memoryStage_interface);
	m_controllockUnit->setMemoryExceptionInterface(m_memoryStage_exceptionStage_interface);
	m_controllockUnit->setExceptionWritebackInterface(m_exceptionStage_writebackStage_interface);
}

core::~core()
{
	delete m_sregister;

	delete m_fetchStage;
	delete m_decodeStage;
	delete m_registerAccessStage;
	delete m_executeStage;
	delete m_memoryStage;
	delete m_exceptionStage;
	delete m_writebackStage;

	delete m_datalockUnit;
	delete m_controllockUnit;

	delete m_fetchStage_decodeStage_interface;
	delete m_decodeStage_registerAccessStage_interface;
	delete m_registerAccessStage_executeStage_interface;
	delete m_executeStage_memoryStage_interface;
	delete m_memoryStage_exceptionStage_interface;
	delete m_exceptionStage_writebackStage_interface;
}

extern int verbose;
void core::simulateOneCycle()
{
	if(verbose == 2)
	{
		dumpPipelineLatches();
		// int a[] = {1, 2};
		// m_sregister->miniDumpRegisterFile(a, 2);
	}

	bool anyDataHazard = m_datalockUnit->anyDataHazard();
	

	m_writebackStage->simulateOneCycle();
	m_exceptionStage->simulateOneCycle();
	m_dcache->simulateOneCycle();
	m_memoryStage->simulateOneCycle();
	m_executeStage->simulateOneCycle();

	if(anyDataHazard == false)
	{
		m_registerAccessStage->simulateOneCycle();
		m_decodeStage->simulateOneCycle();
		m_icache->simulateOneCycle();
		m_fetchStage->simulateOneCycle();
	}

	if(m_fetchStage->isMainEndAddressReached() && allLatchesEmpty())//TODO currently assuming a single core system
	{
		setSimulationDone(true);
		if(verbose != 0)
		{
			m_sregister->dumpRegisterFile();
			m_containingProcessor->getAttachedMemory()->dumpMemory(0xfffffed0, 0xfffffffe);
			
			m_dcache->printCache();
			m_icache->printCache();
		}
	}
}

bool core::allLatchesEmpty()
{
	if(m_fetchStage->getPCWaitingFor() != 0xffffffff)
		return false;
	if(m_fetchStage_decodeStage_interface->peekElementsPendingMessage(getDecodeStage()) != 0)
		return false;
	if(m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(getRegisterAccessStage()) != 0)
		return false;
	if(m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(getExecuteStage()) != 0)
		return false;
	if(m_executeStage_memoryStage_interface->peekElementsPendingMessage(getMemoryStage()) != 0)
		return false;
	if(m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(getExceptionStage()) != 0)
		return false;
	if(m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(getWritebackStage()) != 0)
		return false;

	return true;
}

std::string* core::getStatistics()
{
	string* stats = new string();
	stats->append("number of instructions = ");
	stats->append(std::to_string(m_writebackStage->getNumberOfInstructions()));
	stats->append("\nnumber of cycles = ");
	stats->append(std::to_string(getClock()));
	stats->append("\ninstructions per cycle = ");
	stats->append(std::to_string((float)m_writebackStage->getNumberOfInstructions()/getClock()));
	stats->append("\nnumber of branches = ");
	stats->append(std::to_string(m_executeStage->getNumberOfBranches()));
	stats->append("\nnumber of taken branches = ");
	stats->append(std::to_string(m_executeStage->getNumberOfTakenBranches()));
	stats->append("\nnumber of loads = ");
	stats->append(std::to_string(m_memoryStage->getNumberOfLoads()));
	stats->append("\nnumber of stores = ");
	stats->append(std::to_string(m_memoryStage->getNumberOfStores()));
	stats->append("\nI-Cache");
	stats->append("\nnumber of hits = ");
	stats->append(std::to_string(m_icache->getHits()));
	stats->append("\nhit ratio = ");
	stats->append(std::to_string(m_icache->getHitRatio()));
	stats->append("\nD-Cache");
	stats->append("\nnumber of hits = ");
	stats->append(std::to_string(m_dcache->getHits()));
	stats->append("\nhit ratio = ");
	stats->append(std::to_string(m_dcache->getHitRatio()));
	stats->append("\n");
	return stats;
}

processor* core::getContainingProcessor()
{
	return m_containingProcessor;
}

fetchStage* core::getFetchStage()
{
	return m_fetchStage;
}

decodeStage* core::getDecodeStage()
{
	return m_decodeStage;
}

registerAccessStage* core::getRegisterAccessStage()
{
	return m_registerAccessStage;
}

executeStage* core::getExecuteStage()
{
	return m_executeStage;
}

memoryStage* core::getMemoryStage()
{
	return m_memoryStage;
}

exceptionStage* core::getExceptionStage()
{
	return m_exceptionStage;
}

writebackStage* core::getWritebackStage()
{
	return m_writebackStage;
}

registerfile* core::getRegisterFile()
{
	return m_sregister;
}

controllockUnit* core::getControlLockUnit()
{
	return m_controllockUnit;
}

cache* core::getIcache()
{
	return m_icache;
}

cache* core::getDcache()
{
	return m_dcache;
}

void core::dumpPipelineLatches()
{
	cout << "[" << getClock() << "]\t\t";
	if(m_fetchStage_decodeStage_interface->doesElementHaveAnyPendingMessage(m_decodeStage))
	{
		cout << hex << "IF-D=" << ((fetchStagedecodeStageMessage*)m_fetchStage_decodeStage_interface->peekElementsPendingMessage(m_decodeStage))->getDPC() << "\t" << dec;
	}
	else
	{
		cout << "IF-D=X\t\t";
	}

	if(m_decodeStage_registerAccessStage_interface->doesElementHaveAnyPendingMessage(m_registerAccessStage))
	{
		cout << hex << "D-RA=" << ((decodeStageregisterAccessStageMessage*)m_decodeStage_registerAccessStage_interface->peekElementsPendingMessage(m_registerAccessStage))->getRPC() << "\t" << dec;
	}
	else
	{
		cout << "D-RA=X\t\t";
	}

	if(m_registerAccessStage_executeStage_interface->doesElementHaveAnyPendingMessage(m_executeStage))
	{
		cout << hex << "RA-EX=" << ((registerAccessStageexecuteStageMessage*)m_registerAccessStage_executeStage_interface->peekElementsPendingMessage(m_executeStage))->getEPC() << "\t" << dec;
	}
	else
	{
		cout << "RA-EX=X\t\t";
	}

	if(m_executeStage_memoryStage_interface->doesElementHaveAnyPendingMessage(m_memoryStage))
	{
		cout << hex << "EX-M=" << ((executeStagememoryStageMessage*)m_executeStage_memoryStage_interface->peekElementsPendingMessage(m_memoryStage))->getMPC() << "\t" << dec;
	}
	else
	{
		cout << "EX-M=X\t\t";
	}

	if(m_memoryStage_exceptionStage_interface->doesElementHaveAnyPendingMessage(m_exceptionStage))
	{
		cout << hex << "M-X=" << ((memoryStageexceptionStageMessage*)m_memoryStage_exceptionStage_interface->peekElementsPendingMessage(m_exceptionStage))->getXPC() << "\t" << dec;
	}
	else
	{
		cout << "M-X=X\t\t";
	}

	if(m_exceptionStage_writebackStage_interface->doesElementHaveAnyPendingMessage(m_writebackStage))
	{
		cout << hex << "X-W=" << ((exceptionStagewritebackStageMessage*)m_exceptionStage_writebackStage_interface->peekElementsPendingMessage(m_writebackStage))->getWPC() << "\t" << dec;
	}
	else
	{
		cout << "X-W=X\t\t";
	}
	cout << endl;
}

void core::dumpTextSegment()
{
	addrType text_start_addr = getTextSegmentStartAddress();
	addrType text_size = getTextSegmentSize();
	m_containingProcessor->getAttachedMemory()->dumpMemory(text_start_addr, text_start_addr + text_size - 1);
}

void core::dumpDataSegment()
{
	addrType data_start_addr = getDataSegmentStartAddress();
	addrType data_size = getDataSegmentSize();
	m_containingProcessor->getAttachedMemory()->dumpMemory(data_start_addr, data_start_addr + data_size - 1);
}

void core::dumpBSSSegment()
{
	addrType bss_start_addr = getBSSSegmentStartAddress();
	addrType bss_size = getBSSSegmentSize();
	m_containingProcessor->getAttachedMemory()->dumpMemory(bss_start_addr, bss_start_addr + bss_size - 1);
}

void core::dumpStackSegment()
{
	addrType stack_start_addr = m_sregister->getRegister(14);
	m_containingProcessor->getAttachedMemory()->dumpMemory(stack_start_addr, MAX_MEM);
}

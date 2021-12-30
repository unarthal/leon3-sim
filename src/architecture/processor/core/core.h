#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_CORE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_CORE_H_

#include "architecture/element.h"

class processor;
class fetchStage;
class decodeStage;
class registerAccessStage;
class executeStage;
class memoryStage;
class exceptionStage;
class writebackStage;
class registerfile;
class datalockUnit;
class controllockUnit;
class cache;

class core: public element
{
private:
	processor* m_containingProcessor;
	fetchStage* m_fetchStage;
	decodeStage* m_decodeStage;
	registerAccessStage* m_registerAccessStage;
	executeStage* m_executeStage;
	memoryStage* m_memoryStage;
	exceptionStage* m_exceptionStage;
	writebackStage* m_writebackStage;
	cache* m_icache;
	cache* m_dcache;
	
	registerfile *m_sregister;

	datalockUnit* m_datalockUnit;
	controllockUnit* m_controllockUnit;

	interface* m_fetchStage_decodeStage_interface;
	interface* m_decodeStage_registerAccessStage_interface;
	interface* m_registerAccessStage_executeStage_interface;
	interface* m_executeStage_memoryStage_interface;
	interface* m_memoryStage_exceptionStage_interface;
	interface* m_exceptionStage_writebackStage_interface;

	

	interface* m_icache_fetchStage_interface;
	interface* m_fetchStage_icache_interface;

	interface* m_dcache_memoryStage_interface;
	interface* m_memoryStage_dcache_interface;


	bool allLatchesEmpty();

public:
	core(processor* x_containingProcessor, int x_numberOfRegisterWindows);
	~core();
	void simulateOneCycle();
	std::string* getStatistics();
	processor* getContainingProcessor();
	fetchStage* getFetchStage();
	decodeStage* getDecodeStage();
	registerAccessStage* getRegisterAccessStage();
	executeStage* getExecuteStage();
	memoryStage* getMemoryStage();
	exceptionStage* getExceptionStage();
	writebackStage* getWritebackStage();
	registerfile* getRegisterFile();
	controllockUnit* getControlLockUnit();
	void dumpPipelineLatches();
	void dumpTextSegment();
	void dumpDataSegment();
	void dumpBSSSegment();
	void dumpStackSegment();

	cache* getIcache();
	cache* getDcache();
};

#endif

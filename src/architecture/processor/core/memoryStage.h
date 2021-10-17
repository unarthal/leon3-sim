#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_MEMORYSTAGE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_MEMORYSTAGE_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/instruction.h"
#include "architecture/processor/core/exceptionStage.h"

class core;
class executeStagememoryStageMessage;
class memoryStageexceptionStageMessage;

class memoryStage : public element
{
private:
	core* m_containingCore;
	interface* m_executeStage_memoryStage_interface;
	interface* m_memoryStage_exceptionStage_interface;
	exceptionStage* m_exceptionStage;
	bool m_waitingForMemoryResponse;
	bool m_hasTrapOccurred;

	counterType pmc_loads;
	counterType pmc_stores;

	void processMessageFromExecuteStage();
	void processMessageFromMemory();

public:
	memoryStage(core* x_containingCore);
	~memoryStage();
	void simulateOneCycle();
	void setHasTrapOccurred();
	std::string* getStatistics();
	core* getContainingCore();
	counterType getNumberOfLoads();
	counterType getNumberOfStores();
	void setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface);
	void setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface);
};

class memoryStageexceptionStageMessage : public message
{
private:
	instruction* m_x_inst;
	unsigned int m_x_pc;

public:
	int retVal;

	//for the XC stage
	bool xc_isError;
	int xc_psr;
	int xc_tbr;
	int xc_nWP;
	addrType xc_PC;
	addrType xc_nPC;

	//for the RW stage
	bool rw_isIntReg;
	bool rw_isFltReg;
	bool rw_isASR;
	bool rw_isPSR;
	bool rw_isFSR;
	bool rw_isTBR;
	bool rw_isY;
	bool rw_isWIM;
	bool rw_isControl;
	bool rw_isTrapSaveCounter;
	    /////////////////////////////
	regType rw_RD;
	regType rw_RD_val;
	regType rw_nextRD;
	regType rw_nextRD_val;
	int rw_F_nWords;
	regType rw_F_RD1;
	regType rw_F_RD2;
	regType rw_F_RD3;
	regType rw_F_RD4;
	regType rw_F_RD1_val;
	regType rw_F_RD2_val;
	regType rw_F_RD3_val;
	regType rw_F_RD4_val;
	addrType rw_PC;
	addrType rw_nPC;
	int rw_PSR;
	int rw_FSR;
	int rw_TBR;
	int rw_Y;
	int rw_WIM;

	memoryStageexceptionStageMessage(element* x_producer, element* x_consumer, instruction* x_x_inst, unsigned int x_x_pc);
	~memoryStageexceptionStageMessage();
	instruction* getXInst();
	unsigned int getXPC();
};

#endif

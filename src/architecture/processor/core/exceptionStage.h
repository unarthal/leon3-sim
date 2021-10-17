#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_EXCEPTIONSTAGE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_EXCEPTIONSTAGE_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/instruction.h"
#include "architecture/processor/core/writebackStage.h"

class core;
class memoryStageexceptionStageMessage;
class exceptionStagewritebackStageMessage;

class exceptionStage : public element
{
private:
	core* m_containingCore;
	interface* m_memoryStage_exceptionStage_interface;
	interface* m_exceptionStage_writebackStage_interface;
	writebackStage* m_writebackStage;
	addrType m_newPCAfterTrap;

	int trap(int x_memAddr, memoryStageexceptionStageMessage* x_x_msg, exceptionStagewritebackStageMessage* x_w_msg);

public:
	exceptionStage(core* x_containingCore);
	~exceptionStage();
	void simulateOneCycle();
	std::string* getStatistics();
	core* getContainingCore();
	addrType getNewPCAfterTrap();
	void setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface);
	void setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface);
};

class exceptionStagewritebackStageMessage : public message
{
private:
	instruction* m_w_inst;
	unsigned int m_w_pc;

public:
	bool isregWrite;
	int retVal;

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

	exceptionStagewritebackStageMessage(element* x_producer, element* x_consumer, instruction* x_w_inst, unsigned int x_w_pc);
	~exceptionStagewritebackStageMessage();
	instruction* getWInst();
	unsigned int getWPC();
};

#endif

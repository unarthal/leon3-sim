#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_EXECUTESTAGE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_EXECUTESTAGE_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/instruction.h"
#include "architecture/processor/core/memoryStage.h"
#include<queue>
#include<vector>

class core;
class registerAccessStageexecuteStageMessage;
class executeStagememoryStageMessage;
class executioncompleteevent;

class executeStage : public element
{
private:
	core* m_containingCore;
	interface* m_registerAccessStage_executeStage_interface;
	interface* m_executeStage_memoryStage_interface;
	memoryStage* m_memoryStage;
	bool m_multicycleExecutionInProgress;
	bool m_hasTrapOccurred;
	int m_wimMask;

	std::priority_queue<executioncompleteevent*, std::vector<executioncompleteevent*>, eventcompare>* m_eventQueue;
	int multiplyLatency;
	int divideLatency;

	counterType pmc_branch;
	counterType pmc_takenBranch;

	void executeInstruction();
	inline int addOverflow(regType regRS1, regType reg_or_imm, regType regRD);
	inline int subtractOverflow(regType regRS1, regType reg_or_imm, regType regRD);
	inline int addCarry(regType regRS1, regType reg_or_imm, regType regRD);
	inline int subtractCarry(regType regRS1, regType reg_or_imm, regType regRD);
	int LoadIntegerInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int LoadFloatingPointInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int StoreIntegerInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int StoreFloatInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int AtomicLoadStoreUnsignedByte(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int SWAP(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void SethiNop(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void LogicalInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void ShiftInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void AddInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int TaggedAddInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void SubtractInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int TaggedSubtractInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void MultiplyStepInstruction(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void MultiplyInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int DivideInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int SaveAndRestoreInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void BranchIntegerInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int BranchFloatInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void CallInstruction(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int JumpAndLink(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int RETT(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int TrapOnICC(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int ReadStateRegisterInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int WriteStateRegisterInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int FpopInstructions(registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void updateICCAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int op, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	int taggedAddSubtract(regType regRS1, regType reg_or_imm, regType regRD, int isTVOpcode, int op, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void updateICCMulLogical(regType regRD, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);
	void updateICCDiv(regType regRD, int isOverflow, registerAccessStageexecuteStageMessage* e_msg, executeStagememoryStageMessage* m_msg);

public:
	executeStage(core* x_containingCore);
	~executeStage();
	void simulateOneCycle();
	void setHasTrapOccurred();
	std::string* getStatistics();
	core* getContainingCore();
	void setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface);
	void setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface);

	counterType getNumberOfBranches();
	counterType getNumberOfTakenBranches();
};

class executeStagememoryStageMessage : public message
{
private:
	instruction* m_m_inst;
	unsigned int m_m_pc;

public:
	bool isMemIns;
	int retVal;

	addrType m_addr;
	bool isInt;
	bool isFloat;
	bool isAtomic;
	bool isLoad;
	bool isStore;
	bool isSwap;

	/// to Store
	int regRD;
	int regNextRD;

	int FSR;

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

	executeStagememoryStageMessage(element* x_producer, element* x_consumer, instruction* x_m_inst, unsigned int x_m_pc);
	~executeStagememoryStageMessage();
	instruction* getMInst();
	unsigned int getMPC();
};

class executioncompleteevent : public event
{
public:
	executioncompleteevent(clockType x_eventTime);
	~executioncompleteevent();
};

#endif

#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_REGISTERACCESSSTAGE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_REGISTERACCESSSTAGE_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/instruction.h"
#include "architecture/processor/core/executeStage.h"

class core;

class registerAccessStage : public element
{
private:
	core* m_containingCore;
	interface* m_decodeStage_registerAccessStage_interface;
	interface* m_registerAccessStage_executeStage_interface;
	executeStage* m_executeStage;
	bool m_hasTrapOccurred;

public:
	registerAccessStage(core* x_containingCore);
	~registerAccessStage();
	void simulateOneCycle();
	void setHasTrapOccurred();
	std::string* getStatistics();
	core* getContainingCore();
	void setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStage_interface);
	void setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface);
};

class registerAccessStageexecuteStageMessage : public message
{
private:
	instruction* m_e_inst;
	unsigned int m_e_pc;

public:
	//TODO cleanup required. all these signals need not be passed from Register Access stage to Execute stage. grip.pdf  figure 189 shows only 'e_inst', 'e_pc', 'rs1', 'operand2'.
	//most of these signals should be internal to register access stage.
	//similar cleanup required in all the stages.

	////Integer
	int l_regRS1;
	int l_reg_or_imm;
	int l_regRS2;
	int l_regRD; //common
	int l_regNextRD; //common

	////Floating Point
	int l_regFs1_1;
	int l_regFs1_2;
	int l_regFs1_3;
	int l_regFs1_4;
	int l_regFs2_1;
	int l_regFs2_2;
	int l_regFs2_3;
	int l_regFs2_4;

	int l_PSR_EF;
	int l_PSR_ET;
	int l_PSR_S;
	int l_PSR_PS;

	int l_WIM;
	int l_PSR;
	int l_FSR;
	int l_ASR;
	int l_TBR;
	int l_Y;

	int l_PSR_icc_N;
	int l_PSR_icc_Z;
	int l_PSR_icc_V;
	int l_PSR_icc_C;
	int l_FSR_RD;

	int l_CWP;
	int l_nWP;

	int l_FSR_fcc;
	int l_PC;
	int l_nPC;
	//  int l_ASRreg;

	//for the XC stage
	bool xc_isError;
	int xc_psr;
	int xc_tbr;
	int xc_nWP;
	addrType xc_PC;
	addrType xc_nPC;

	registerAccessStageexecuteStageMessage(element* x_producer, element* x_consumer, instruction* x_e_inst, unsigned int x_e_pc);
	~registerAccessStageexecuteStageMessage();
	instruction* getEInst();
	unsigned int getEPC();
};

#endif

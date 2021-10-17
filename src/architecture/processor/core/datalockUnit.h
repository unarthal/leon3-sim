#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_DATALOCKUNIT_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_DATALOCKUNIT_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/instruction.h"
#include "architecture/processor/core/executeStage.h"

class core;

class datalockUnit
{
private:
	core* m_containingCore;
	interface* m_decodeStage_registerAccessStage_interface;
	interface* m_registerAccessStage_executeStage_interface;
	interface* m_executeStage_memoryStage_interface;
	interface* m_memoryStage_exceptionStage_interface;
	interface* m_exceptionStage_writebackStage_interface;

	bool isRAWHazard(int x_sourceReg);
	bool anyWriterToConditionCodes();
	bool anyCWPModifyingInstruction();

public:
	datalockUnit(core* x_containingCore);
	~datalockUnit();

	bool anyDataHazard();
	core* getContainingCore();
	void setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStage_interface);
	void setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface);
	void setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface);
	void setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface);
	void setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface);
};

#endif

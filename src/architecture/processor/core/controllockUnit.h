#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_CONTROLLOCKUNIT_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_CONTROLLOCKUNIT_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/instruction.h"
#include "architecture/processor/core/executeStage.h"

class core;

class controllockUnit
{
private:
	core* m_containingCore;
	interface* m_fetchStage_decodeStage_interface;
	interface* m_decodeStage_registerAccessStage_interface;
	interface* m_registerAccessStage_executeStage_interface;
	interface* m_executeStage_memoryStage_interface;
	interface* m_memoryStage_exceptionStage_interface;
	interface* m_exceptionStage_writebackStage_interface;

public:
	controllockUnit(core* x_containingCore);
	~controllockUnit();

	bool anyControlHazard();
	void performAnnulment(addrType x_PC_to_be_annuled);
	core* getContainingCore();
	void setFetchDecodeInterface(interface* x_fetchStage_decodeStage_interface);
	void setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStage_interface);
	void setRegisterAccessExecuteInterface(interface* x_registerAccessStage_executeStage_interface);
	void setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface);
	void setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface);
	void setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface);
};

#endif

#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_WRITEBACKSTAGE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_WRITEBACKSTAGE_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/registers/registerfile.h"
#include "architecture/processor/core/instruction.h"
//#include "architecture/processor/core/fetchStage.h"

class core;
class exceptionStagewritebackStageMessage;
class writebackStagefetchStageMessage;

class writebackStage : public element
{
private:
	core* m_containingCore;
	registerfile* sregister;
	interface* m_exceptionStage_writebackStage_interface;

	counterType pmc_instructions;

public:
	writebackStage(core* x_containingCore);
	~writebackStage();
	void simulateOneCycle();
	std::string* getStatistics();
	core* getContainingCore();
	void setExceptionWritebackInterface(interface* x_exceptionStage_writebackStage_interface);
	counterType getNumberOfInstructions();
};

#endif

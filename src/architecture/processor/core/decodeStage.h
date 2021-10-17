#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_DECODESTAGE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_DECODESTAGE_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/instruction.h"
#include "architecture/processor/core/registerAccessStage.h"

class core;

class decodeStage : public element
{
private:
	core* m_containingCore;
	interface* m_fetchStage_decodeStage_interface;
	interface* m_decodeStage_registerAccessStage_interface;
	registerAccessStage* m_registerAccessStage;
	bool m_hasTrapOccurred;

public:
	decodeStage(core* x_containingCore);
	~decodeStage();
	void simulateOneCycle();
	void setHasTrapOccurred();
	std::string* getStatistics();
	core* getContainingCore();
	void setFetchDecodeInterface(interface* x_fetchStage_decodeStage_interface);
	void setDecodeRegisterAccessInterface(interface* x_decodeStage_registerAccessStage_interface);
};

class decodeStageregisterAccessStageMessage : public message
{
private:
	instruction* m_r_inst; //includes r_imm, as specified in fig 189 of the grip document
	unsigned int m_r_pc;
public:
	decodeStageregisterAccessStageMessage(element* x_producer, element* x_consumer, instruction* x_r_inst, unsigned int x_r_pc);
	~decodeStageregisterAccessStageMessage();
	instruction* getRInst();
	unsigned int getRPC();
};

#endif

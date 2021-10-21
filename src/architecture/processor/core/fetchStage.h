#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_FETCHSTAGE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_FETCHSTAGE_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include "architecture/processor/core/decodeStage.h"

class core;

class fetchStage : public element
{
private:
	core* m_containingCore;
	interface* m_fetchStage_decodeStage_interface;
	decodeStage* m_decodeStage;
	bool m_hasTrapOccurred;
	addrType m_PCwaitingFor;
	bool m_mainEndAddressReached;

public:
	fetchStage(core* x_containingCore);
	~fetchStage();
	void simulateOneCycle();
	void setHasTrapOccurred();
	addrType getPCWaitingFor();
	bool isMainEndAddressReached();
	void setPCWaitingFor(addrType x_PCwaitingFor);
	std::string* getStatistics();
	core* getContainingCore();
	void setFetchDecodeInterface(interface* x_fetchStage_decodeStage_interface);
};

class fetchStagedecodeStageMessage : public message
{
private:
	char* m_d_inst;
	addrType m_d_pc;
public:
	fetchStagedecodeStageMessage(element* x_producer, element* x_consumer, char* x_d_inst, addrType x_d_pc);
	~fetchStagedecodeStageMessage();
	char* getDInst();
	addrType getDPC();
};

#endif

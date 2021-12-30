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

	interface* m_fetchStage_icache_interface;
	interface* m_icache_fetchStage_interface;

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
	
	interface* getIcacheFetchStageInterface();
	void setIcacheFetchStageInterface(interface* x_icache_fetchstage_interface);

	interface* getFetchStageIcacheInterface();
	void setFetchStageIcacheInterface(interface* x_fetchstage_icache_interface);
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

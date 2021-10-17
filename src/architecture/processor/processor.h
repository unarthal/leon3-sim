#ifndef SRC_ARCHITECTURE_PROCESSOR_PROCESSOR_H_
#define SRC_ARCHITECTURE_PROCESSOR_PROCESSOR_H_

#include "architecture/element.h"
#include "architecture/interface.h"
#include <vector>

using namespace std;

class core;
class memory;

class processor: public element
{
private:
	interface* m_processor_memory_interface;
	interface* m_memory_processor_interface;
	memory* m_attachedMemory;

	vector<core*>* m_cores;
	//TODO add other processor components like AHB bus

public:
	processor();
	processor(memory* x_mem, int x_numberOfCores, int x_numberOfRegisterWindows);
	~processor();
	memory* getAttachedMemory();
	void simulateOneCycle();
	std::string* getStatistics();
	vector<core*>* getCores();
	core* getCore(int x_coreIndex);
	interface* getProcessorMemoryInterface();
	interface* getMemoryProcessorInterface();
};

#endif

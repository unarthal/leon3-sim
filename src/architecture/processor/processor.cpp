#include "architecture/processor/processor.h"
#include "architecture/processor/core/core.h"
#include "architecture/memory/memory.h"

processor::processor()
{
	m_processor_memory_interface = 0;
	m_memory_processor_interface = 0;
	m_attachedMemory = 0;
	m_cores = 0;
}

processor::processor(memory* x_mem, int x_numberOfCores, int x_numberOfRegisterWindows) : element()
{
	m_processor_memory_interface = new interface();
	m_memory_processor_interface = new interface();
	m_attachedMemory = x_mem;
	getOutputInterfaces()->push_back(m_processor_memory_interface);
	getInputInterfaces()->push_back(m_memory_processor_interface);
	x_mem->setMemoryProcessorInterface(m_memory_processor_interface);
	x_mem->setProcessorMemoryInterface(m_processor_memory_interface);
	x_mem->getInputInterfaces()->push_back(m_processor_memory_interface);
	x_mem->getOutputInterfaces()->push_back(m_memory_processor_interface);

	m_cores = new vector<core*>();
	for(int i = 0; i < x_numberOfCores; i++)
	{
		m_cores->push_back(new core(this, x_numberOfRegisterWindows));
	}
}

processor::~processor()
{
	for(unsigned int i = 0; i < m_cores->size(); i++)
	{
		delete m_cores->at(i);
		m_cores->at(i) = 0;
	}
	delete m_cores;

	delete m_processor_memory_interface;
	delete m_memory_processor_interface;
}

memory* processor::getAttachedMemory()
{
	return m_attachedMemory;
}

void processor::simulateOneCycle()
{
	for(unsigned int i = 0; i < m_cores->size(); i++)
	{
		m_cores->at(i)->simulateOneCycle();
	}
	//TODO simulate AHB bus and other processor components
}

std::string* processor::getStatistics()
{
	string* stats = new string();
	stats->append("\nPROCESSOR\n\n");
	for(unsigned int i = 0; i < m_cores->size(); i++)
	{
		stats->append("core ");
		stats->append(std::to_string(i));
		stats->append("\n");
		stats->append(*(m_cores->at(i)->getStatistics()));
		stats->append("\n\n");
	}
	return stats;
}

vector<core*>* processor::getCores()
{
	return m_cores;
}

core* processor::getCore(int x_coreIndex)
{
	return m_cores->at(x_coreIndex);
}

interface* processor::getProcessorMemoryInterface()
{
	return m_processor_memory_interface;
}

interface* processor::getMemoryProcessorInterface()
{
	return m_memory_processor_interface;
}

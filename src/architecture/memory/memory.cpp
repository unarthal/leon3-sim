#include "architecture/memory/memory.h"

/*
 * Returns 1, if memoryAddress is not aligned on <alignment> boundary.
 * Returns 0, if memoryAddress is aligned on <alignment> boundary.
 * It's useful to prevent loading mis-aligned memoryAddress
 * in double word (DWORD) instructions by raising
 * memory_address_not_aligned traps.
 */
int is_mem_address_not_aligned(unsigned int memoryAddress, int alignment)
{
    switch(alignment)
    {
        case 2: if((((unsigned int)(memoryAddress / 2)) * 2) == memoryAddress) return 0; else return 1;
        case 4: if((((unsigned int)(memoryAddress / 4)) * 4) == memoryAddress) return 0; else return 1;
        case 8: if((((unsigned int)(memoryAddress / 8)) * 8) == memoryAddress) return 0; else return 1;
    }
    return 0;
}

memory::memory() : element()
{
	m_memory_processor_interface = 0;
	m_processor_memory_interface = 0;

	pmc_loads = 0;
	pmc_stores = 0;
}

memory::~memory()
{

}

interface* memory::getMemoryProcessorInterface()
{
	return m_memory_processor_interface;
}

void memory::setMemoryProcessorInterface(interface* x_memory_processor_interface)
{
	m_memory_processor_interface = x_memory_processor_interface;
	getOutputInterfaces()->push_back(m_memory_processor_interface);
}

interface* memory::getProcessorMemoryInterface()
{
	return m_processor_memory_interface;
}

void memory::setProcessorMemoryInterface(interface* x_processor_memory_interface)
{
	m_processor_memory_interface = x_processor_memory_interface;
	getInputInterfaces()->push_back(m_processor_memory_interface);
}

memorymessage::memorymessage(element* x_producer, element* x_consumer, memorymessagetype x_messageType, addrType x_address, int x_noOfBytes, char* x_value) : message(x_producer, x_consumer)
{
	m_messageType = x_messageType;
	m_address = x_address;
	m_noOfBytes = x_noOfBytes;
	m_value = x_value;
}

memorymessage::~memorymessage()
{

}

memorymessagetype memorymessage::getMemoryMessageType()
{
	return m_messageType;
}

void memorymessage::setMemoryMessageType(memorymessagetype x_memoryMessageType)
{
	m_messageType = x_memoryMessageType;
}

addrType memorymessage::getAddress()
{
	return m_address;
}

void memorymessage::setAddress(addrType x_address)
{
	m_address = x_address;
}

int memorymessage::getNoOfBytes()
{
	return m_noOfBytes;
}

void memorymessage::setNoOfBytes(int x_noOfBytes)
{
	m_noOfBytes = x_noOfBytes;
}

char* memorymessage::getValue()
{
	return m_value;
}

void memorymessage::setValue(char* x_value)
{
	m_value = x_value;
}

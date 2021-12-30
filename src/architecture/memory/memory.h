#ifndef SRC_ARCHITECTURE_MEMORY_MEMORY_H_
#define SRC_ARCHITECTURE_MEMORY_MEMORY_H_

/*
 * TODO to split functionality into memory and memory controller
 */

#include "architecture/constants_typedefs.h"
#include "architecture/element.h"
#include "architecture/interface.h"

int is_mem_address_not_aligned(unsigned int memoryAddress, int alignment);

class memory : public element
{
private:
	interface* m_processor_memory_interface;
	interface* m_memory_processor_interface;

protected:
	counterType pmc_loads;
	counterType pmc_stores;

public:
	memory();
	~memory();
	void simulateOneCycle(){}
	counterType getNumberOfLoads();
	counterType getNumberOfStores();
	interface* getMemoryProcessorInterface();
	void setMemoryProcessorInterface(interface* x_memory_processor_interface);
	interface* getProcessorMemoryInterface();
	void setProcessorMemoryInterface(interface* x_processor_memory_interface);
	virtual void setByte(addrType byteAddress, char byteValue) = 0; //to be used only during state initialization
	//virtual int setWord(addrType memoryAddress, addrType word) = 0; //to be used only during state initialization
	virtual void dumpMemory(addrType x_startAddr, addrType x_endAddr) = 0;
};

enum memorymessagetype {Read, ReadResponse, Write, WriteResponse};

class memorymessage : public message
{
private:
	memorymessagetype m_messageType;
	addrType m_address;
	int m_noOfBytes;
	char* m_value;

	int m_orgBytes;
	int m_offset;
	addrType org_address;
public:
	memorymessage(element* x_producer, element* x_consumer, memorymessagetype x_messageType, addrType x_address, int x_noOfBytes, char* x_value);
	~memorymessage();
	memorymessagetype getMemoryMessageType();
	void setMemoryMessageType(memorymessagetype x_memoryMessageType);
	addrType getAddress();
	void setAddress(addrType x_address);
	int getNoOfBytes();
	void setNoOfBytes(int x_noOfBytes);
	char* getValue();
	void setValue(char* x_value);

	int getOrgNoOfBytes();
	void setOrgNoOfBytes(int x_value);
	int getOffset();
	void setOffset(int x_value);
	addrType getOrgAddress();
	void setOrgAddress(addrType x_value);


};

#endif

#include "architecture/memory/simple_memory.h"
#include "generic/utility.h"
#include <iostream>
#include "architecture/system.h"
#include "elf/elf.h"

using namespace std;

simple_memory::simple_memory(clockType x_latency) : memory()
{
	m_memoryArray = new char[(unsigned int)MAX_MEM];
	m_latency = x_latency;
	m_eventQueue = new std::priority_queue<simplememoryevent*, std::vector<simplememoryevent*>, eventcompare>();
}

simple_memory::~simple_memory()
{
	delete(m_memoryArray);
	delete(m_eventQueue);
}

void simple_memory::setByte(addrType byteAddress, char byteValue) //to be used only during state initialization
{
	m_memoryArray[byteAddress] = byteValue;
}

char* simple_memory::getBytes(addrType x_memoryAddress, int x_numberOfBytes)
{
	char* value = new char[x_numberOfBytes];
	for(int i = 0; i < x_numberOfBytes; i++)
	{
		value[i] = m_memoryArray[x_memoryAddress+i];
	}
	return value;
}

void simple_memory::setBytes(addrType x_memoryAddress, int x_numberOfBytes, char* x_value)
{
	for(int i = 0; i < x_numberOfBytes; i++)
	{
		m_memoryArray[x_memoryAddress+i] = x_value[i];
	}
	delete x_value;
}

void simple_memory::dumpMemory(addrType x_startAddr, addrType x_endAddr)
{
	cout << "\n[BEGIN] memory dump" << endl;
	for(addrType i = x_startAddr; i <= x_endAddr; i++)
	{
		cout << hex << i << " : " << (((unsigned int)m_memoryArray[i])&0xff) << dec << endl;
	}
	cout << "[END] memory dump" << endl;
}

void simple_memory::miniDumpMemory()
{
	cout << "\n[BEGIN] memory dump" << endl;
	for(addrType i = 0xfffffc18; i < 0xfffffeb8; i++)
	{
		cout << hex << i << " : " << (((unsigned int)m_memoryArray[i])&0xff) << dec << endl;
	}
	cout << "[END] memory dump" << endl;
}

void simple_memory::processEventQueue()
{
	while(m_eventQueue->empty() == false
				&& m_eventQueue->top()->getEventTime() <= getClock()
				&& getMemoryProcessorInterface()->getBusy() == false)
	{
		simplememoryevent* evt = m_eventQueue->top();
		addrType addr = evt->getMemoryMessage()->getAddress();
		int noOfBytes = evt->getMemoryMessage()->getNoOfBytes();
		char* value = evt->getMemoryMessage()->getValue();
		element* originalProducer = evt->getMemoryMessage()->getProducer();

		evt->getMemoryMessage()->setProducer(this);
		evt->getMemoryMessage()->setConsumer(originalProducer);

		switch(evt->getMemoryMessage()->getMemoryMessageType())
		{
		case Read:
			value = getBytes(addr, noOfBytes);
			//reuse the same message
			evt->getMemoryMessage()->setMemoryMessageType(ReadResponse);
			evt->getMemoryMessage()->setValue(value);
			pmc_loads++;
			break;

		case Write:
			setBytes(addr, noOfBytes, value);
			evt->getMemoryMessage()->setMemoryMessageType(WriteResponse);
			evt->getMemoryMessage()->setValue(0);
			pmc_stores++;
			break;

		default:
			cout << "ERROR: received event type " << evt->getMemoryMessage()->getMemoryMessageType() << " in simplememory" << endl;
			showErrorAndExit("UNKNOWN EVENT RECEIVED");
		}

		getMemoryProcessorInterface()->addPendingMessage(evt->getMemoryMessage());
		getProcessorMemoryInterface()->setBusy(false);//trying a simple model with no pipelining
		//std::cout << dec << "[" << getClock() << "] MEM RESP: " << evt->getMemoryMessage()->getMemoryMessageType() << " : " << hex << evt->getMemoryMessage()->getAddress() << dec << "\n";

		m_eventQueue->pop();
		delete evt; //remember to delete messages and events once their work is done!
	}
}

void simple_memory::simulateOneCycle()
{
	processEventQueue();

	while(getProcessorMemoryInterface()->getBusy() == false /*memory is not busy*/
			&& getProcessorMemoryInterface()->doesElementHaveAnyPendingMessage(this) == true /*memory has pending work to do*/)
	{
		memorymessage *msg = (memorymessage*)(getProcessorMemoryInterface()->popElementsPendingMessage(this));
		m_eventQueue->push(new simplememoryevent(msg, getClock() + m_latency - 1));
		getProcessorMemoryInterface()->setBusy(true);//trying a simple model with no pipelining
		//std::cout << dec << "[" << getClock() << "] MEM REQ: " << msg->getMemoryMessageType() << " : " << hex << msg->getAddress() << dec << "\n";

		if(m_latency == 1)
		{
			processEventQueue();
		}
	}
}

std::string* simple_memory::getStatistics()
{
	string* stats = new string();
	stats->append("MEMORY\n\n");
	stats->append("\nnumber of loads = ");
	stats->append(std::to_string(getNumberOfLoads()));
	stats->append("\nnumber of stores = ");
	stats->append(std::to_string(getNumberOfStores()));
	stats->append("\n");
	return stats;
}

counterType simple_memory::getNumberOfLoads()
{
	return pmc_loads;
}

counterType simple_memory::getNumberOfStores()
{
	return pmc_stores;
}

simplememoryevent::simplememoryevent(memorymessage* x_msg, clockType x_eventTime)
{
	m_msg = x_msg;
	setEventTime(x_eventTime);
}

simplememoryevent::~simplememoryevent()
{

}

memorymessage* simplememoryevent::getMemoryMessage()
{
	return m_msg;
}

void simplememoryevent::setMemoryMessage(memorymessage* x_msg)
{
	m_msg = x_msg;
}



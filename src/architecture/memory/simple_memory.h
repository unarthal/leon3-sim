#ifndef SRC_ARCHITECTURE_MEMORY_SIMPLEMEMORY_H_
#define SRC_ARCHITECTURE_MEMORY_SIMPLEMEMORY_H_

#include "architecture/memory/memory.h"
#include<queue>
#include<vector>
#include<functional>

enum simplememoryeventtype {LoadComplete, StoreComplete};

class simplememoryevent : public event
{
private:
	memorymessage* m_msg;
public:
	simplememoryevent(memorymessage* x_msg, clockType x_eventTime);
	~simplememoryevent();
	memorymessage* getMemoryMessage();
	void setMemoryMessage(memorymessage* x_msg);
};

class simple_memory : public memory
{
private:
	char* m_memoryArray;
	clockType m_latency;
	std::priority_queue<simplememoryevent*, std::vector<simplememoryevent*>, eventcompare>* m_eventQueue;
	char* getBytes(addrType x_memoryAddress, int x_numberOfBytes);
	void setBytes(addrType x_memoryAddress, int x_numberOfBytes, char* x_value);
	void processEventQueue();
	void miniDumpMemory();

public:
	simple_memory(clockType x_latency);
	~simple_memory();
	void simulateOneCycle();
	std::string* getStatistics();
	counterType getNumberOfLoads();
	counterType getNumberOfStores();
	void dumpMemory(addrType x_startAddr, addrType x_endAddr);

	void setByte(addrType memoryAddress, char byteValue); //to be used outside of simple_memory.cpp only during state initialization
	//int setWord(addrType memoryAddress, char* word); //to be used outside of simple_memory.cpp only during state initialization
};

#endif

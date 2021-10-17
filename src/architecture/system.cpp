#include "architecture/system.h"
#include "architecture/memory/memory.h"
#include "architecture/memory/simple_memory.h"
#include "config/config.h"
#include "elf/elf.h"
#include "architecture/element.h"
#include <vector>
#include "architecture/processor/processor.h"
#include <iostream>
#include <fstream>

using namespace std;

clockType g_clock;
clockType getClock()
{
	return g_clock;
}
void setClock(clockType x_clock)
{
	g_clock = x_clock;
}
void incrementClock()
{
	g_clock++;
}

bool g_simulationDone;
bool getSimulationDone()
{
	return g_simulationDone;
}
void setSimulationDone(bool x_simulationDone)
{
	g_simulationDone = x_simulationDone;
}

vector<element*>* architecture_elements;
//TODO vectors of cores, TLBs, caches, interconnects, memories, device interfaces, and pipes between modules

void initializeArchitecture(string executablePath, string configFilePath)
{
	cout << hex << "\n[BEGIN] instantiating and initializing architectural components" << endl;

	g_clock = 0;
	g_simulationDone = false;

	architecture_elements = new vector<element*>();

	//TODO select memory implementation based on config file
	memory* mem = new simple_memory(xmlReader_generalInt(configFilePath, "<Latency>", "</Latency>"));
	architecture_elements->push_back(mem);
	initializeMemory(configFilePath, executablePath, mem);

	int numberOfRegisterWindows = xmlReader_generalInt(configFilePath, "<NWindows>", "</NWindows>");
	/*
	 * TODO sparcv8 executable already contains a value for nwindows.
	 * this has been used to initialize the relevant bytes of the simulated memory.
	 * should we change these memory bytes to numberOfRegisterWindows?
	mem->setWord(OFFSET-4, numberOfRegisterWindows-1);
	 */
	processor *proc = new processor(mem, 1/*parameterized number of cores*/, numberOfRegisterWindows);
	architecture_elements->push_back(proc);

	cout << "\n[END] instantiating and initializing architectural components" << dec << endl;
}

void simulate()
{
	cout << hex << "\n[BEGIN] simulation\n" << endl;

	while(getSimulationDone() == false)
	{
		for(unsigned int i = 0; i < architecture_elements->size(); i++)
		{
			architecture_elements->at(i)->simulateOneCycle();
		}
		incrementClock();
	}

	cout << "\n[END] simulation" << endl;
}

string* getSystemState()
{
	return 0;
//	invoke getElementState() of constituent elements and collate
//	return element state
}

string* getSystemStatistics()
{
	string* stats = new string();
	for(unsigned int i = 0; i < architecture_elements->size(); i++)
	{
		stats->append(*(architecture_elements->at(i)->getStatistics()));
		stats->append("\n\n");
	}
	return stats;
}

void recordSystemStatistics(string statisticsFilePath)
{
	string* stats = getSystemStatistics();
	ofstream out(statisticsFilePath);
	out << *stats;
	out.close();
}

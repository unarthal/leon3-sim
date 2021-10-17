#ifndef SRC_ARCHITECTURE_SYSTEM_H_
#define SRC_ARCHITECTURE_SYSTEM_H_

#include<string>
#include "architecture/constants_typedefs.h"

void initializeArchitecture(std::string executablePath, std::string configFilePath);
void simulate();
std::string* getSystemState();
std::string* getSystemStatistics();
void recordSystemStatistics(std::string statisticsFilePath);
clockType getClock();
void setClock(clockType x_clock);
void incrementClock();
bool getSimulationDone();
void setSimulationDone(bool x_simulationDone);

#endif /* SRC_ARCHITECTURE_SYSTEM_H_ */

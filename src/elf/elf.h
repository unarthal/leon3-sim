#ifndef SRC_ELF_ELF_H_
#define SRC_ELF_ELF_H_

#include <string>
#include "../architecture/constants_typedefs.h"
#include "architecture/memory/memory.h"

void createDump(std::string l_filename);
void findStartAndEndOfTextSegment(std::string executablePath);
int initializeLoader(char const*elfBinary);
void load_sparc_instructions(std::string elfBinary, memory *memory);
void initializeMemory(std::string confileFilePath, std::string executablePath, memory *mem);
addrType getMainStartAddress();
addrType getMainEndAddress();
addrType getTextSegmentStartAddress();
addrType getTextSegmentSize();
addrType getDataSegmentStartAddress();
addrType getDataSegmentSize();
addrType getBSSSegmentStartAddress();
addrType getBSSSegmentSize();

#endif /* SRC_ELF_ELF_H_ */

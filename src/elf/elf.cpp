#include "elf/elf.h"

#include <iostream>
#include <fstream>
#include "generic/utility.h"
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "config/config.h"

using namespace std;

addrType main_start_addr= -1;
addrType main_end_addr=-1;
addrType text_start_addr = -1;
addrType text_size = -1;
addrType data_start_addr = -1;
addrType data_size = -1;
addrType bss_start_addr = -1;
addrType bss_size = -1;
extern int verbose;

void createDump(string configFilePath, string executablePath){
	string cmd = xmlReader_generalString(configFilePath, "<Disassembler>", "</Disassembler>");
	cmd += " -D ";
	cmd += executablePath;
	cmd += " > dump";
	if(system(cmd.c_str()) > 0)
	{
		showErrorAndExit("disassembly failed");
	}
}

void findStartAndEndOfTextSegment(){

	ifstream l_file;
    l_file.open("dump");
    string l_identify_main[2];
    l_file >> l_identify_main[0];

    int l_count = 0;
    while(!l_file.eof()){
        if(l_identify_main[l_count].find("<main>:")!=string::npos){
            break;
        }
        l_count = 1 - l_count;
        l_file >> l_identify_main[l_count];
    }
    l_count = 1 - l_count;

    main_start_addr=convertHexToInt(l_identify_main[l_count]);

    while(!l_file.eof()){
        if(l_identify_main[l_count].find(">:")!=string::npos){
            break;
        }
        l_count = 1 - l_count;
        l_file >> l_identify_main[l_count];
    }
    l_count = 1 - l_count;

    main_end_addr=convertHexToInt(l_identify_main[l_count])-8;//skipping the retl and nop

    l_file.close();

    if(verbose != 0)
    {
    	cout << hex << "main() starting address: " << main_start_addr <<"\nmain() ending address: "<< main_end_addr << dec << endl;
    }
}

struct loadedSections
{
	char sectionName[15];
	unsigned long sectionLoadAddress;
	short sectionType;
	unsigned long sectionSize;
	unsigned long instructionCount;
	struct loadedSections* nextSection;
};

int initializeLoader(char const*elfBinary){
    static int fileDescriptor = RET_FAILURE;
	if(elfBinary == NULL)
	{
		close(fileDescriptor);
		return -1;
	}
	else
	{
		if((fileDescriptor = open(elfBinary, O_RDWR)) == RET_FAILURE)
			return ELF_FILE_DOES_NOT_EXIST_ERROR;
		else
			return fileDescriptor;
	}

}

void load_sparc_instructions(string elfBinary, memory *memory)
{
	GElf_Ehdr elf_header;		    // ELF header
	GElf_Shdr shdr;                 // Section Header
	Elf_Scn* scn = NULL;            // Section Descriptor
	Elf_Data* sectionData;          // Data Descriptor
	unsigned long instructionCount;
	int fileDescriptor;
	struct loadedSections* elfSections, *elfSectionsPrevPtr, *elfSectionCurPtr;

	instructionCount = 0;
	elfSections = (struct loadedSections*)malloc(sizeof(struct loadedSections));
	if(elfSections == NULL) //NOT ABLE TO ASSIGN LOCATION USING MALLOC? CHANGE TO NEW?
	{
		elfSections->sectionType = ELF_SECTION_LOAD_ERROR;
		showErrorAndExit("parsing SPARC executable: ELF_SECTION_LOAD_ERROR");
	}

	//char hi[65];
    //sprintf(hi,"%p",elf_header.e_entry );
	//cout<<"Entry : "<<hi<<"\n";

	elfSectionCurPtr = elfSections;

	fileDescriptor = initializeLoader(elfBinary.c_str());
	if(fileDescriptor == ELF_FILE_DOES_NOT_EXIST_ERROR)
	{
		elfSections->sectionType = ELF_FILE_DOES_NOT_EXIST_ERROR;
		showErrorAndExit("parsing SPARC executable: ELF_FILE_DOES_NOT_EXIST_ERROR");
	}

	// Check libelf version
	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		elfSections->sectionType = ELF_BINARY_OUT_OF_DATE;
		showErrorAndExit("parsing SPARC executable: ELF_BINARY_OUT_OF_DATE");
	}

	Elf* elf = elf_begin(fileDescriptor, ELF_C_READ, NULL);
	if(elf == NULL)
	{
		elfSections->sectionType = ELF_POINTER_INITIALIZATION_ERROR;
		showErrorAndExit("parsing SPARC executable: ELF_POINTER_INITIALIZATION_ERROR");
	}

	gelf_getehdr(elf, &elf_header);

	// Iterate over section headers
	addrType current_memory_pointer;

	while((scn = elf_nextscn(elf, scn)) != 0)
	{

		gelf_getshdr(scn, &shdr);
		size_t sectionDataByteCounter;

		if(((shdr.sh_flags & SHF_EXECINSTR) && (shdr.sh_flags & SHF_ALLOC)) ||  // .text section
			((shdr.sh_flags & SHF_WRITE) && (shdr.sh_flags & SHF_ALLOC)))		// .data & .bss sections
		{
			sectionData = NULL;
			sectionDataByteCounter = 0;
			unsigned long sectionLoadAddress = shdr.sh_addr;
			current_memory_pointer = sectionLoadAddress;
			while((sectionDataByteCounter < shdr.sh_size) && ((sectionData = elf_getdata (scn, sectionData)) != NULL))
			{
				char* sectionDataBuffer = (char*)sectionData -> d_buf;

				if(sectionDataBuffer == NULL)
					break;
				while (sectionDataBuffer < (char*)sectionData-> d_buf + sectionData -> d_size )
				{
					memory->setByte(current_memory_pointer, *sectionDataBuffer);
					sectionLoadAddress++;
					sectionDataBuffer++;
					current_memory_pointer++;
					sectionDataByteCounter++;
				}
			}

			if((shdr.sh_flags & SHF_EXECINSTR) && (shdr.sh_flags & SHF_ALLOC))
				instructionCount = sectionDataByteCounter/4;

			// Store section information
			strcpy(elfSectionCurPtr->sectionName, elf_strptr(elf, elf_header.e_shstrndx, shdr.sh_name));
			elfSectionCurPtr->sectionLoadAddress = shdr.sh_addr;
			if(sectionData != NULL)
				elfSectionCurPtr->sectionSize = sectionData->d_size;
			else
				elfSectionCurPtr->sectionSize = 0;
			elfSectionCurPtr->instructionCount = instructionCount;
			if((shdr.sh_flags & SHF_EXECINSTR) && (shdr.sh_flags & SHF_ALLOC))
				elfSectionCurPtr->sectionType = CODE_SECTION;
			else
				elfSectionCurPtr->sectionType = DATA_SECTION;
			elfSectionCurPtr->nextSection = (struct loadedSections*)malloc(sizeof(struct loadedSections));
			if(elfSectionCurPtr->nextSection == NULL)
			{
				elfSections->sectionType = ELF_SECTION_LOAD_ERROR;
				showErrorAndExit("parsing SPARC executable: ELF_SECTION_LOAD_ERROR");
			}

			if(verbose != 0)
			{
				cout << "section name: " << elfSectionCurPtr->sectionName << endl;
				cout << "starting address: " << hex << elfSectionCurPtr->sectionLoadAddress << endl;
				cout << "size: " << elfSectionCurPtr->sectionSize << dec << endl;
			}

			if(strcmp(elfSectionCurPtr->sectionName, ".text") == 0)
			{
				text_start_addr = elfSectionCurPtr->sectionLoadAddress;
				text_size = elfSectionCurPtr->sectionSize;
			}
			if(strcmp(elfSectionCurPtr->sectionName, ".data") == 0)
			{
				data_start_addr = elfSectionCurPtr->sectionLoadAddress;
				data_size = elfSectionCurPtr->sectionSize;
			}
			if(strcmp(elfSectionCurPtr->sectionName, ".bss") == 0)
			{
				bss_start_addr = elfSectionCurPtr->sectionLoadAddress;
				bss_size = elfSectionCurPtr->sectionSize;
			}
			elfSectionsPrevPtr = elfSectionCurPtr;
			elfSectionCurPtr = elfSectionCurPtr->nextSection;
		}
	}
	//memory->setMemDumpMax(current_memory_pointer); /// last byte addr to dump into mem_dump.txt

	free(elfSectionCurPtr);
	elfSectionsPrevPtr->nextSection = NULL;

	/* TODO to check if sections other than the text section are being loaded in the memory */
}

void deleteDump(){
	string cmd = "rm dump";
	system(cmd.c_str());
}

void initializeMemory(string confileFilePath, string executablePath, memory *mem)
{
	cout << hex << "\n[BEGIN] initializing memory" << endl;

	createDump(confileFilePath, executablePath);
	findStartAndEndOfTextSegment();
	load_sparc_instructions(executablePath, mem);
	deleteDump();

	cout << "[END] initializing memory" << dec << endl;
}

addrType getMainStartAddress()
{
	return main_start_addr;
}

addrType getMainEndAddress()
{
	return main_end_addr;
}

addrType getTextSegmentStartAddress()
{
	return text_start_addr;
}

addrType getTextSegmentSize()
{
	return text_size;
}

addrType getDataSegmentStartAddress()
{
	return data_start_addr;
}

addrType getDataSegmentSize()
{
	return data_size;
}

addrType getBSSSegmentStartAddress()
{
	return bss_start_addr;
}

addrType getBSSSegmentSize()
{
	return bss_size;
}

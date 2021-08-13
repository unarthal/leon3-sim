#include "reader.h"
#include "header.h"
//#include "register.h"

int Reader::initializeLoader(char *elfBinary){
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



struct loadedSections* Reader::load_sparc_instructions(char *elfBinary, Memory *memory, int NWindows)
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
		return elfSections;
	}
	// printf("Entry : %p\n",elf_header.e_entry);
	char hi[65];
    sprintf(hi,"%p",elf_header.e_entry );
	xout<<"Entry : "<<hi<<"\n";
	xout << "A\n";
	elfSectionCurPtr = elfSections;
	xout << "A\n";
	fileDescriptor = initializeLoader(elfBinary);
	if(fileDescriptor == ELF_FILE_DOES_NOT_EXIST_ERROR)
	{
		elfSections->sectionType = ELF_FILE_DOES_NOT_EXIST_ERROR;
		return elfSections;
	}
	xout << "B\n";
	// Check libelf version
	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		elfSections->sectionType = ELF_BINARY_OUT_OF_DATE;
		return elfSections;
	}

	Elf* elf = elf_begin(fileDescriptor, ELF_C_READ, NULL);
	if(elf == NULL)
	{
		elfSections->sectionType = ELF_POINTER_INITIALIZATION_ERROR;
		return elfSections;
	}

	gelf_getehdr(elf, &elf_header);
	
	// Iterate over section headers
	unsigned int current_memory_pointer = OFFSET;
	xout << "C\n";		
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
			while((sectionDataByteCounter < shdr.sh_size) && ((sectionData = elf_getdata (scn, sectionData)) != NULL))
			{	
				char* sectionDataBuffer = (char*)sectionData -> d_buf;
				
				if(sectionDataBuffer == NULL)
					break;
				while (sectionDataBuffer < (char*)sectionData-> d_buf + sectionData -> d_size )
				{
					//xout << current_memory_pointer << " CURRENT MEM" << endl;
					
					//printf("WASTE %02X\n", (unsigned int)*sectionDataBuffer);
					//xout<<" ";
					//xout << "WASTE " << (unsigned int)*sectionDataBuffer << endl;
					
					memory->setByte(current_memory_pointer, *sectionDataBuffer); 
					sectionLoadAddress++;
					sectionDataBuffer++;
					current_memory_pointer++;
					
					// printf("%02X", (unsigned int)*sectionDataBuffer);
					// xout<<" ";
					memory->setByte(current_memory_pointer, *sectionDataBuffer);
					sectionLoadAddress++;
					sectionDataBuffer++;
					current_memory_pointer++;
					// printf("%02X", (unsigned int)*sectionDataBuffer);
					// xout<<" ";
					memory->setByte(current_memory_pointer, *sectionDataBuffer);
					sectionLoadAddress++;
					sectionDataBuffer++;
					current_memory_pointer++;
					// printf("%02X", (unsigned int)*sectionDataBuffer);
					// xout<<" "<<endl;
					memory->setByte(current_memory_pointer, *sectionDataBuffer);
					sectionLoadAddress++;
					sectionDataBuffer++;
					current_memory_pointer++;

					sectionDataByteCounter += 4;
					instructionCount++;
				}
			}

			xout << "D\n";
			// Store section information
			strcpy(elfSectionCurPtr->sectionName, elf_strptr(elf, elf_header.e_shstrndx, shdr.sh_name));
			elfSectionCurPtr->sectionLoadAddress = shdr.sh_addr;
			// printf("Section Name : %s\n",elfSectionCurPtr->sectionName);
			xout<<"Section Name : %s\n"<<elfSectionCurPtr->sectionName<<"\n";
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
				return elfSections;
			}
			elfSectionsPrevPtr = elfSectionCurPtr;
			elfSectionCurPtr = elfSectionCurPtr->nextSection;
		}
	}
	MEMDUMP_MAX = current_memory_pointer; /// last byte addr to dump into mem_dump.txt
	
	free(elfSectionCurPtr);
	elfSectionsPrevPtr->nextSection = NULL;
	memory->setWord(1073760824, NWindows-1);
	return elfSections;
}





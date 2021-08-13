#include "simulator.h"
#include <fstream>
#include <iostream>
extern int MEM_LAT;
extern int MEMDUMP_MAX;



void displayIns(instruction* i){   //This function prints the instruction with all the different fields
    xout<<"op: "<<i->op<<"\t";
    xout<<"dis30: "<<i->disp30<<"\t";
    xout<<"rd: "<<i->rd<<"\t";
    xout<<"op2: "<<i->op2<<"\t";
    xout<<"a: "<<i->a<<"\t";
    xout<<"cond: "<<i->cond<<"\t";
    xout<<"op3: "<<i->op3<<"\t";
    xout<<"rs1: "<<i->rs1<<"\t";
    xout<<"i: "<<i->i<<"\t";
    xout<<"rs2: "<<i->rs2<<"\t";
    xout<<"asi: "<<i->asi<<"\t";
    xout<<"simm13: "<<i->simm13<<"\t";
    xout<<"opf: "<<i->opf<<"\t";
    xout<<"imm22: "<<i->imm22<<"\t";
	xout<<"disp22: "<<i->disp22<<"\t";
    xout<<"\n";
	
}

// int trap(int memAddr, Register *sregister, Memory* memory){
// 	if(sregister->SRegisters.psr->getEt()==0)
// 		return error_mode;
// 	sregister->SRegisters.psr->setEt(0);
// 	sregister->SRegisters.psr->setPs(sregister->SRegisters.psr->getS());
// 	sregister->SRegisters.psr->setS(1);
// 	xout << "intrap getocwp:" <<sregister->SRegisters.psr->getCwp() << endl;
// 	sregister->SRegisters.psr->setCwp((sregister->SRegisters.psr->getCwp()-1)%sregister->SRegisters.totalregisterWindows);
// 	if((sregister->SRegisters.psr->getCwp())<0)
// 		sregister->SRegisters.psr->setCwp((sregister->SRegisters.psr->getCwp())+sregister->SRegisters.totalregisterWindows);
// 	xout << "intrap getncwp:" <<sregister->SRegisters.psr->getCwp() << endl;
	
// 	sregister->setRegister(17, sregister->getPC());
// 	sregister->setRegister(18, sregister->getnPC());
// 	sregister->SRegisters.tbr->setTt(memAddr);
// 	sregister->setPC(sregister->SRegisters.tbr->getTbr());
// 	sregister->setnPC(sregister->SRegisters.tbr->getTbr()+4);
// 	xout << "TRAP PC:" << sregister->getPC() << endl;
// 	return RET_SUCCESS;
// }

Simulator::Simulator(char* f){
    elfBinary = f;  //We give this ELF file as input to the simulator
    reader = new Reader();
    memory = new Memory();
    sregister = new Register();

	MEM_Interface = new memoryInterface(memory, NULL); // producer, consumer=NULL for 1:many

	fede_latch = new FE_DE_latch();
	dera_latch = new DE_RA_latch();
	raex_latch = new RA_EX_latch();
	exme_latch = new EX_ME_latch();
	mexc_latch = new ME_XC_latch();
	xcrw_latch = new XC_RW_latch(); 
	rwfe_latch = new RW_FE_latch();

	stage_FE = new FE(sregister, fede_latch, rwfe_latch);
	stage_DE = new DE(fede_latch, dera_latch);
	stage_RA = new RA(sregister ,raex_latch, dera_latch);
	stage_EX = new EX( raex_latch, exme_latch);
	stage_ME = new ME( exme_latch, mexc_latch);
	stage_XC = new XC( mexc_latch, xcrw_latch);
	stage_RW = new RW(sregister, xcrw_latch, rwfe_latch);

	memory->MEM_Interface = MEM_Interface;
	stage_FE->MEM_Interface = MEM_Interface;
	stage_ME->MEM_Interface = MEM_Interface;




}

void Simulator::startSimulation(int start_addr, int end_addr, int NWindows){
    struct loadedSections *elfSectionsPrevPtr, *elfSectionCurPtr;
    elfSectionCurPtr = reader->load_sparc_instructions(this->elfBinary, this->memory, NWindows); //Reading from the ELF file and storing in the memory
	//This is an inbuilt utility in c++.
    
	xout << "ALL INSTRUCTIONS LOADED"<< endl;
	do
	{
		xout<<"Disassembly of Section: "<< elfSectionCurPtr->sectionName <<"\n";
		xout<<"Section Load Address: "<<elfSectionCurPtr->sectionLoadAddress<<"\n";
		xout<<"Section Size: "<<elfSectionCurPtr->instructionCount<<"\n";

		xout<<"Section Type: "<< elfSectionCurPtr->sectionType <<"\n";

		elfSectionsPrevPtr = elfSectionCurPtr;
		free(elfSectionsPrevPtr);
		elfSectionCurPtr = elfSectionCurPtr->nextSection;
	} while(elfSectionCurPtr != NULL);
	
	xout<<"\n";
    
	int memory_point = start_addr;//-OFFSET; //This is the PC that we got from main()
	int last_inst=end_addr;//-OFFSET; //This is the last supposed to be PC
	ofstream memdump_file;
	memdump_file.open("mem_dump.txt");
	
	sregister->initializeRegisters(NWindows);
	// Initializing execution environment
	sregister->setPC(memory_point);
	sregister->setnPC(memory_point+4);
	
 	
	unsigned long instructionCount;
	int c=0,ret,i=0;
	//TODO write fns to write to a file
	//This while loop reads and executes the instructions
	
	///Initiallize sim globals
	clock_cycles = 0;
	SIM_LOOP_SIGNAL = RET_SUCCESS;

	while(sregister->getPC()!=last_inst)
	{
		xout<<"\nPC: before execution of instruction: "<<sregister->getPC()<<endl;

		char* cpuInstruction;
		instruction* decodedInstruction ;
		
		xout <<"INSTRUCTION@PC: " << memory->getWord(sregister->getPC())<<endl;


		stage_FE->perform();
		MEM_Interface->processRequests();

		cpuInstruction = fede_latch->Ins;

		stage_DE->perform();
		decodedInstruction = &dera_latch->decodedIns;	

	   

		xout<<"Hexadecimal: ";
		displayWord(cpuInstruction, 1);		
		xout<<endl;
		displayIns(decodedInstruction);
		xout <<"EXECUTION STARTED" <<endl;

		stage_RA->perform(); 

		stage_EX->perform();

		stage_ME->perform();
		MEM_Interface->processRequests();


		stage_XC->perform();

		stage_RW->perform();

		if (SIM_LOOP_SIGNAL == -1){
			xout << "ERROR MODE "<<endl;
			break;	
		}


		xout <<"EXECUTION COMPLETE" <<endl;
		

		
		if(Quiet != 2){
		xout<<"PROCESSOR STATE ELEMENTS-------------------------------------------------------------------\n\n";
		sregister->display_All_Register();
		xout<<"----------------------------------------------------------------------------\n";
		}

		i++;
		


		delete [] cpuInstruction;

		xout<<"No of instructions executed = "<<nINS<<endl;
	
		clock_cycles++;
		printMemory_to_file(OFFSET, MEMDUMP_MAX, *memory, memdump_file);
	
		xout<<"##Cycles: "<<clock_cycles<<"\n";
		xout<<"###################------------------------------------xxxx------------------------------------###################\n";

	}

	generateStats();
	memdump_file.close();
}


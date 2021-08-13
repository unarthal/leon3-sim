#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "generic/reader.h"
#include "memory/memory.h"
#include "processor/registers/register.h"
#include "processor/instruction.h"
#include "generic/utility.h"
#include "memory/memory_checks.h"
#include "generic/header.h"

#include "processor/pipeline/FE.h"
#include "processor/pipeline/DE.h"
#include "processor/pipeline/RA.h"
#include "processor/pipeline/EX.h"
#include "processor/pipeline/ME.h"
#include "processor/pipeline/XC.h"
#include "processor/pipeline/RW.h"

class Simulator{
    private:
        char* elfBinary;
        Reader *reader;
        Memory *memory;
        Register *sregister;
    
    public:
    memoryInterface* MEM_Interface ; // producer, consumer=NULL for 1:many

    
    FE_DE_latch* fede_latch ;
	DE_RA_latch* dera_latch ;
	RA_EX_latch* raex_latch ;
	EX_ME_latch* exme_latch ;
	ME_XC_latch* mexc_latch ;
	XC_RW_latch* xcrw_latch ; 
	RW_FE_latch* rwfe_latch ;

	FE* stage_FE ;
	DE* stage_DE ;
	RA* stage_RA ;
	EX* stage_EX ;
	ME* stage_ME ;
	XC* stage_XC ;
	RW* stage_RW ;
    

    Simulator(char*);
    void startSimulation(int, int, int);
};

#endif

#ifndef REGISTER_H
#define REGISTER_H

#include <cstdlib>
#include "psr.h"
#include "fsr.h"
#include "processor/constants.h"
#include "tbr.h"


#define REGISTER_WINDOW_WIDTH    		16
#define GLOBAL_REGISTERS       		 	 8
//#define REGISTER_WINDOWS       		 	 4    //NWINDOWS
#define SIZEOF_INTEGER_REGISTER  	 	 4


struct registers
{
       regType* registerSet;
       regType* globalRegisters;
       
       regType y, wim;
       
       addrType pc, npc;
       
       int totalregisterWindows;   //NWINDOWS
       
       regType asrRegisters[32];
       regType floatingPointRegisters[32];
       regType CoprocessorRegisters[32];

       trap_base_register*  tbr;
       processor_status_register* psr;
       floating_point_state_register* fsr;
       //class processor_status_register __attribute__ ((aligned (8))) psr;

};

class Register{
    public:
        struct registers SRegisters;
        
        void initializeRegisters(int NWindows);
        
        void setRegisterWindow(int registerWindow);
        regType getRegisterWindow();// * Returns the current window pointer value contained in CWP field of PSR.
        /*
 * Sets the current window pointer value contained in CWP field of PSR.
 * CWP, being a field 5 bit wide, can vary in the range [0-32]. Specific 
 * implementation can cut the maximum limit short by specifying REGISTER_WINDOWS.
 * Also adjusts current window pointer address (SRegisters.cwptr) 
 * held in memory. SRegisters.registerSet is the base address of register 
 * set and CWP multiplied by REGISTER_WINDOW_WIDTH gives the offset.
 */
        
        void setY(regType val);
        regType getY();
        
        int* getWindowPointer(int direction);/*
 * Returns the array pointer to current register window forward or backward
 * as specified by direction. Positive direction indicates
 * forward move and negative direction indicates backward move.
 * Wrap around is done, if needed.
 * SAVE and RESTORE SPARC instructions require the pointer to
 * register window to be shifted appropriately.
 */
        
        regType getRegister(unsigned int regNum);
        int setRegister(unsigned int regNum, regType registerValue);
        
        int setFloatingRegister(unsigned int regNum, regType registerValue);
        regType getFloatingRegister(unsigned int regNum);
        
        int setCoprocessorRegister(unsigned int regNum, regType registerValue);
        regType getCoprocessorRegister(unsigned int regNum);
        
        void setPC(addrType p);
        addrType getPC();
        
        void setnPC(addrType p);
        addrType getnPC();

       void setASRRegister(int, int);
       int getASRRegister(int);
        
        void display_All_Register();
};

#endif

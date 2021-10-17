#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_REGISTERS_REGISTERFILE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_REGISTERS_REGISTERFILE_H_

#include "architecture/processor/core/registers/fsr.h"
#include "architecture/processor/core/registers/psr.h"
#include "architecture/processor/core/registers/tbr.h"
#include "architecture/constants_typedefs.h"
#include "architecture/element.h"

struct registers
{
       regType* registerSet;
       regType* globalRegisters;
       
       regType y, wim;
       
       addrType pc, npc;
       
       int totalRegisterWindows;   //NWINDOWS
       
       regType asrRegisters[32];
       regType floatingPointRegisters[32];
       regType CoprocessorRegisters[32];

       trap_base_register*  tbr;
       processor_status_register* psr;
       floating_point_state_register* fsr;
       //class processor_status_register __attribute__ ((aligned (8))) psr;

};

class registerfile : public element{
    public:
        struct registers* SRegisters;
        
        void initializeRegisters(int NWindows, addrType initialPCValue);
        
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
        
        void dumpRegisterFile();

        void simulateOneCycle(){}
        std::string* getStatistics();

        ~registerfile();
};

#endif

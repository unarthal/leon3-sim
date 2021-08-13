#include "register.h"
#include "generic/utility.h"
#include "generic/header.h"

void Register::initializeRegisters(int REGISTER_WINDOWS)
{
	
	SRegisters.totalregisterWindows = REGISTER_WINDOWS;       // get number of register windows from configuration file

    /* SRegisters.registerSet holds the base address of the set of register windows.
     * Each register window consists of a set of IN and LOCAL registers of its own, as well as
     * a set of OUT registers shared from its adjacent window. Hence, there are (IN + LOCAL) = (8 + 8) = 16
     * registers in a window (REGISTER_WINDOW_WIDTH), each register being 4 byte (SIZEOF_INTEGER_REGISTER)
     * wide. Therefore, 16 * 8 * <Number of register windows> bytes has been allocated.
     */
    
	//SRegisters.registerSet = (regType*)malloc(REGISTER_WINDOW_WIDTH * REGISTER_WINDOWS * SIZEOF_INTEGER_REGISTER);
	SRegisters.registerSet = new regType [REGISTER_WINDOW_WIDTH * REGISTER_WINDOWS];
	
    // GLOBAL registers, not being part of of register window, are contained in memory pointed to by SRegisters.globalRegisters.
	//SRegisters.globalRegisters = (regType*)malloc(SIZEOF_INTEGER_REGISTER * GLOBAL_REGISTERS);
	SRegisters.globalRegisters = new regType [GLOBAL_REGISTERS];

    SRegisters.y = 0;
	SRegisters.pc = 0;
	SRegisters.npc = 4;                 // nPC should point to the instruction ahead.
	SRegisters.wim = 1;

	SRegisters.psr = new processor_status_register();
	SRegisters.fsr = new floating_point_state_register();
	SRegisters.tbr = new trap_base_register();
	SRegisters.tbr->setTba(0x40000000);

	SRegisters.psr->setCwp(REGISTER_WINDOWS-1);
	SRegisters.psr->setEf(1);
	SRegisters.psr->setEt(1);
	//Initialize global registers
	for(int count = 0; count < GLOBAL_REGISTERS; count++)
		SRegisters.globalRegisters[count] = 0;

	// Initialize out, local, in registers
	for(int count = 0; count < REGISTER_WINDOW_WIDTH * REGISTER_WINDOWS; count++)
		SRegisters.registerSet[count] = 0;
	
	SRegisters.registerSet[(REGISTER_WINDOWS-2)*16+8+6] = ((long int)MAX_MEM-128);
	// xout << "REGISTER_WINDOWS = "<< REGISTER_WINDOWS <<" "<<MAX_MEM-128 << endl;
	// xout << "SRegisters.registerSet[(REGISTER_WINDOWS-2)*16+8+6]=" << SRegisters.registerSet[(REGISTER_WINDOWS-2)*16+8+6] << endl;
	
	SRegisters.registerSet[(REGISTER_WINDOWS-1)*16+8+6] = MAX_MEM;
	
	// Initialize ASR
	
	for(int count = 0; count < 32; count++)
		SRegisters.asrRegisters[count] = 0;
        
    // Initialize floating point registers
	for(int count = 0; count < 32; count++)
		SRegisters.floatingPointRegisters[count] = 0;
	
	for(int count = 0; count < 32; count++)
		SRegisters.CoprocessorRegisters[count] = 0;
}

/*
 * Returns the current window pointer value contained in CWP field of PSR.
 */
regType Register::getRegisterWindow()
{
    return SRegisters.psr->getCwp();
}

void Register::setY(regType val){
	SRegisters.y = val;
}
regType Register::getY(){
	return SRegisters.y ;
}
/*
 * Returns the array pointer to current register window forward or backward
 * as specified by direction. Positive direction indicates
 * forward move and negative direction indicates backward move.
 * Wrap around is done, if needed.
 * SAVE and RESTORE SPARC instructions require the pointer to
 * register window to be shifted appropriately.
 */
int* Register::getWindowPointer(int direction)
{
   // Move window pointer forward
   if(direction == 1)
    {
		
        if(SRegisters.psr->getCwp() == (SRegisters.totalregisterWindows - 1)) //TODO
            return SRegisters.registerSet;
        else
            return SRegisters.registerSet+SRegisters.psr->getCwp()*REGISTER_WINDOW_WIDTH + REGISTER_WINDOW_WIDTH;
    }
	
    // Move window pointer backward
    else
    {
        if(SRegisters.psr->getCwp() == 0)
            return SRegisters.registerSet + (REGISTER_WINDOW_WIDTH * (SRegisters.totalregisterWindows - 1)); //TODO
        else
            return SRegisters.registerSet+SRegisters.psr->getCwp()*REGISTER_WINDOW_WIDTH - REGISTER_WINDOW_WIDTH;
    }
}

/*
 * Sets the current window pointer value contained in CWP field of PSR.
 * CWP, being a field 5 bit wide, can vary in the range [0-32]. Specific 
 * implementation can cut the maximum limit short by specifying REGISTER_WINDOWS.
 * Also adjusts current window pointer address (SRegisters.cwptr) 
 * held in memory. SRegisters.registerSet is the base address of register 
 * set and CWP multiplied by REGISTER_WINDOW_WIDTH gives the offset.
 */
void Register::setRegisterWindow(int registerWindow)
{
    SRegisters.psr->setCwp(registerWindow);
}

/*
locals r[16]-r[23] --> registerSet(0,7)
ins r[24]-r[31] --> registerSet(8,15)
outs r[8]-r[15] --> registerSet(8,15) from previous register window
*/
regType Register::getRegister(unsigned int regNum){
	if(regNum<8){
		return *(SRegisters.globalRegisters + regNum); 
	}
	else if(regNum<16){

		int* previousWindowPointer = getWindowPointer(-1); 
		return *(previousWindowPointer + regNum);                // OUT register are shared with IN registers from previous window.
	}
	else{
		return *(SRegisters.registerSet+REGISTER_WINDOW_WIDTH*(SRegisters.psr->getCwp())+regNum-16);
	}
}

int Register::setRegister(unsigned int regNum, regType registerValue){
	if(regNum<8){
		if(regNum==0)
			return RET_SUCCESS;
		*(SRegisters.globalRegisters + regNum)=registerValue; 
		return RET_SUCCESS;
	}
	else if(regNum<16){

		int* previousWindowPointer = getWindowPointer(-1); 
		*(previousWindowPointer + regNum)=registerValue;                // OUT register are shared with IN registers from previous window.
		return RET_SUCCESS;

	}
	else{
		 *(SRegisters.registerSet+SRegisters.psr->getCwp()*REGISTER_WINDOW_WIDTH+regNum-16)=registerValue;
		 return RET_SUCCESS;
	}

}

int Register::setFloatingRegister(unsigned int regNum, regType registerValue){

	*(SRegisters.floatingPointRegisters + regNum) = registerValue; 
	xout << "&registerValue "<<*(float*)&registerValue<<endl;
	return RET_SUCCESS;
}

regType Register::getFloatingRegister(unsigned int regNum){

	return *(SRegisters.floatingPointRegisters + regNum);
}


int Register::setCoprocessorRegister(unsigned int regNum, regType registerValue){

	*(SRegisters.CoprocessorRegisters + regNum) = registerValue; 
	return RET_SUCCESS;
}

regType Register::getCoprocessorRegister(unsigned int regNum){

	return *(SRegisters.CoprocessorRegisters + regNum);
}

addrType Register::getPC(){
	return SRegisters.pc;

}
addrType Register::getnPC(){
	return SRegisters.npc;

}
void Register::setPC(addrType p){
	SRegisters.pc=p;
}
void Register::setnPC(addrType p){
	SRegisters.npc=p;
}

void Register::setASRRegister(int RNo, int SetVal){
	SRegisters.asrRegisters[RNo]=SetVal;
}
int Register::getASRRegister(int RNo){
	return SRegisters.asrRegisters[RNo];
}


void Register::display_All_Register()
{
	for(int count = 0; count < SRegisters.totalregisterWindows; count++){
		xout<<"REGISTER WINDOW : "<<count<<endl<<endl;
		xout<<"OUTPUT : \t\t";
		for(int j=0;j<8;j++){
			//in
			if(count==0){
				xout<<"o"<<j<<": "<< *(SRegisters.registerSet+16*SRegisters.totalregisterWindows-8+j) <<"\t";
			}
			else xout<<"o"<<j<<": "<< *(SRegisters.registerSet+count*16-8+j) <<"\t";
		}
		xout<<endl;
		xout<<"LOCAL  : \t\t";
		for(int j=0;j<8;j++){
			//local
			xout<<"l"<<j<<": "<< *(SRegisters.registerSet+count*16+j)<<"\t";

		}
		xout<<endl;
		xout<<"INPUT  : \t\t";
		for(int j=0;j<8;j++){
			xout<<"i"<<j<<": "<< *(SRegisters.registerSet+count*16+8+j)<<"\t";

		}
		xout<<endl;
		xout<<"GLOBAL : \t\t";
		for(int j=0;j<8;j++){
			//global
			xout<<"g"<<j<<": "<< *(SRegisters.globalRegisters+j) <<"\t";

		}
		xout<<endl<<endl;
		
	}
	xout << "FLoating point:" << endl;
	for (int i=0;i<32;i++){
		if(i%4==0)
			xout << endl;
		xout << *(float*)(&SRegisters.floatingPointRegisters[i])<<"\t";
	}
	xout << endl << endl;
	xout<<"ASR: ";
	for (int i=0;i<32;i++){
	xout << SRegisters.asrRegisters[i] <<" ";
	}
	xout << endl;
	xout<<"CPR: ";
	for (int i=0;i<32;i++){
	xout << SRegisters.CoprocessorRegisters[i] <<" ";
	}
	xout << endl;


	xout <<"WIM:";disp_int(SRegisters.wim,31,0);
	// decToBinary(SRegisters.wim);
	xout<<"Y:  ";disp_int(SRegisters.y,31,0);
	xout<<"PSR:";disp_int(SRegisters.psr->getPSR(),31,0);
	xout<<"FSR:";disp_int(SRegisters.fsr->getFSR(),31,0);
	xout<<"TBR:";disp_int(SRegisters.tbr->getTbr(),31,0);
	xout << endl;

		char hi[32];
        

	xout<<"PC 0x: ";
	//printf("%02X(%lu)",SRegisters.pc, SRegisters.pc);
	sprintf(hi,"%X",SRegisters.pc );
	xout<<hi<<"("<<SRegisters.pc<<")\n";
	xout << endl;
	xout<<"Next PC 0x: ";
	//printf("%02X(%lu)",SRegisters.npc, SRegisters.npc);
	sprintf(hi,"%X",SRegisters.npc );
	xout<<hi<<"("<<SRegisters.npc<<")\n";
	xout << endl;

}


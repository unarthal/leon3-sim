#include "architecture/processor/core/registers/registerfile.h"

#include "generic/utility.h"
#include <iostream>

using namespace std;

void registerfile::initializeRegisters(int nRegisterWindows, addrType initialPCValue)
{
	SRegisters = new registers();
	SRegisters->totalRegisterWindows = nRegisterWindows;       // get number of register windows from configuration file

    /* SRegisters->registerSet holds the base address of the set of register windows.
     * Each register window consists of a set of IN and LOCAL registers of its own, as well as
     * a set of OUT registers shared from its adjacent window. Hence, there are (IN + LOCAL) = (8 + 8) = 16
     * registers in a window (REGISTER_WINDOW_WIDTH), each register being 4 byte (SIZEOF_INTEGER_REGISTER)
     * wide. Therefore, 16 * 8 * <Number of register windows> bytes has been allocated.
     */
    
	//SRegisters->registerSet = (regType*)malloc(REGISTER_WINDOW_WIDTH * REGISTER_WINDOWS * SIZEOF_INTEGER_REGISTER);
	SRegisters->registerSet = new regType [REGISTER_WINDOW_WIDTH * nRegisterWindows];
	
    // GLOBAL registers, not being part of of register window, are contained in memory pointed to by SRegisters->globalRegisters.
	//SRegisters->globalRegisters = (regType*)malloc(SIZEOF_INTEGER_REGISTER * GLOBAL_REGISTERS);
	SRegisters->globalRegisters = new regType [GLOBAL_REGISTERS];

    SRegisters->y = 0;
	SRegisters->pc = initialPCValue;
	SRegisters->npc = initialPCValue+4;
	SRegisters->wim = 1;

	SRegisters->psr = new processor_status_register();
	SRegisters->fsr = new floating_point_state_register();
	SRegisters->tbr = new trap_base_register();
	SRegisters->tbr->setTba(0x40000000);

	SRegisters->psr->setCwp(nRegisterWindows-1);
	SRegisters->psr->setEf(1);
	SRegisters->psr->setEt(1);
	//Initialize global registers
	for(int count = 0; count < GLOBAL_REGISTERS; count++)
		SRegisters->globalRegisters[count] = 0;

	// Initialize out, local, in registers
	for(int count = 0; count < REGISTER_WINDOW_WIDTH * nRegisterWindows; count++)
		SRegisters->registerSet[count] = 0;
	
	//SRegisters->registerSet[(nRegisterWindows-2)*16+8+6] = ((long int)MAX_MEM-128);//TODO why is this required?
	//SRegisters->registerSet[(nRegisterWindows-1)*16+8+6] = MAX_MEM;//TODO why is this required?
	setRegister(30, MAX_MEM);
	setRegister(14, MAX_MEM-127);//this represents the activation block of the function that invoked main()
	// Initialize ASR
	for(int count = 0; count < 32; count++)
		SRegisters->asrRegisters[count] = 0;
        
    // Initialize floating point registers
	for(int count = 0; count < 32; count++)
		SRegisters->floatingPointRegisters[count] = 0;
	
	for(int count = 0; count < 32; count++)
		SRegisters->CoprocessorRegisters[count] = 0;
}

/*
 * Returns the current window pointer value contained in CWP field of PSR.
 */
regType registerfile::getRegisterWindow()
{
    return SRegisters->psr->getCwp();
}

void registerfile::setY(regType val){
	SRegisters->y = val;
}
regType registerfile::getY(){
	return SRegisters->y ;
}
/*
 * Returns the array pointer to current register window forward or backward
 * as specified by direction. Positive direction indicates
 * forward move and negative direction indicates backward move.
 * Wrap around is done, if needed.
 * SAVE and RESTORE SPARC instructions require the pointer to
 * register window to be shifted appropriately.
 */
int* registerfile::getWindowPointer(int direction)
{
   // Move window pointer forward
   if(direction == 1)
    {
		
        if(SRegisters->psr->getCwp() == (SRegisters->totalRegisterWindows - 1)) //TODO
            return SRegisters->registerSet;
        else
            return SRegisters->registerSet+SRegisters->psr->getCwp()*REGISTER_WINDOW_WIDTH + REGISTER_WINDOW_WIDTH;
    }
	
    // Move window pointer backward
    else
    {
        if(SRegisters->psr->getCwp() == 0)
            return SRegisters->registerSet + (REGISTER_WINDOW_WIDTH * (SRegisters->totalRegisterWindows - 1)); //TODO
        else
            return SRegisters->registerSet+SRegisters->psr->getCwp()*REGISTER_WINDOW_WIDTH - REGISTER_WINDOW_WIDTH;
    }
}

/*
 * Sets the current window pointer value contained in CWP field of PSR.
 * CWP, being a field 5 bit wide, can vary in the range [0-32]. Specific 
 * implementation can cut the maximum limit short by specifying REGISTER_WINDOWS.
 * Also adjusts current window pointer address (SRegisters->cwptr)
 * held in memory. SRegisters->registerSet is the base address of register
 * set and CWP multiplied by REGISTER_WINDOW_WIDTH gives the offset.
 */
void registerfile::setRegisterWindow(int registerWindow)
{
    SRegisters->psr->setCwp(registerWindow);
}

/*
locals r[16]-r[23] --> registerSet(0,7)
ins r[24]-r[31] --> registerSet(8,15)
outs r[8]-r[15] --> registerSet(8,15) from previous register window
*/
regType registerfile::getRegister(unsigned int regNum){
	if(regNum<8){
		return *(SRegisters->globalRegisters + regNum);
	}
	else/* if(regNum<16)*/{

		int index = (SRegisters->psr->getCwp()*REGISTER_WINDOW_WIDTH + regNum - 8)%(REGISTER_WINDOW_WIDTH * SRegisters->totalRegisterWindows);
		return *(SRegisters->registerSet + index);                // OUT register are shared with IN registers from previous window.
	}
	/*else{
		return *(SRegisters->registerSet+REGISTER_WINDOW_WIDTH*(SRegisters->psr->getCwp())+regNum-16);
	}*/
}

int registerfile::setRegister(unsigned int regNum, regType registerValue){
	if(regNum<8){
		if(regNum==0)
			return RET_SUCCESS;
		*(SRegisters->globalRegisters + regNum)=registerValue;
		return RET_SUCCESS;
	}
	else/* if(regNum<16)*/{

		int index = (SRegisters->psr->getCwp()*REGISTER_WINDOW_WIDTH + regNum - 8)%(REGISTER_WINDOW_WIDTH * SRegisters->totalRegisterWindows);
		*(SRegisters->registerSet + index)=registerValue;                // OUT register are shared with IN registers from previous window.
		return RET_SUCCESS;

	}
	/*else{
		 *(SRegisters->registerSet+SRegisters->psr->getCwp()*REGISTER_WINDOW_WIDTH+regNum-16)=registerValue;
		 return RET_SUCCESS;
	}*/

}

int registerfile::setFloatingRegister(unsigned int regNum, regType registerValue){

	*(SRegisters->floatingPointRegisters + regNum) = registerValue;
	cout << "&registerValue "<<*(float*)&registerValue<<endl;
	return RET_SUCCESS;
}

regType registerfile::getFloatingRegister(unsigned int regNum){

	return *(SRegisters->floatingPointRegisters + regNum);
}


int registerfile::setCoprocessorRegister(unsigned int regNum, regType registerValue){

	*(SRegisters->CoprocessorRegisters + regNum) = registerValue;
	return RET_SUCCESS;
}

regType registerfile::getCoprocessorRegister(unsigned int regNum){

	return *(SRegisters->CoprocessorRegisters + regNum);
}

addrType registerfile::getPC(){
	return SRegisters->pc;

}
addrType registerfile::getnPC(){
	return SRegisters->npc;

}
void registerfile::setPC(addrType p){
	SRegisters->pc=p;
}
void registerfile::setnPC(addrType p){
	SRegisters->npc=p;
}

void registerfile::setASRRegister(int RNo, int SetVal){
	SRegisters->asrRegisters[RNo]=SetVal;
}
int registerfile::getASRRegister(int RNo){
	return SRegisters->asrRegisters[RNo];
}


void registerfile::dumpRegisterFile()
{
	cout << hex << "\n[BEGIN] register file dump" << endl;
	cout << "CWP = " << SRegisters->psr->getCwp() << endl;
	for(int count = 0; count < SRegisters->totalRegisterWindows; count++){
		cout<<"REGISTER WINDOW : "<<count<<endl<<endl;
		cout<<"OUTPUT : \t\t";
		for(int j=0;j<8;j++){
			cout<<"o"<<j<<": "<< *(SRegisters->registerSet+count*16+j)<<"\t";
		}
		cout<<endl;
		cout<<"LOCAL  : \t\t";
		for(int j=0;j<8;j++){
			//local
			cout<<"l"<<j<<": "<< *(SRegisters->registerSet+count*16+j+8)<<"\t";

		}
		cout<<endl;
		cout<<"INPUT  : \t\t";
		for(int j=0;j<8;j++){
			//in
			if(count==SRegisters->totalRegisterWindows-1){
				cout<<"i"<<j<<": "<< *(SRegisters->registerSet+j) <<"\t";
			}
			else cout<<"i"<<j<<": "<< *(SRegisters->registerSet+count*16+j+16) <<"\t";
		}
		cout<<endl<<endl;
		
	}
	cout<<"GLOBAL : \t\t";
	for(int j=0;j<8;j++){
		//global
		cout<<"g"<<j<<": "<< *(SRegisters->globalRegisters+j) <<"\t";

	}
	cout<<endl<<endl;
	cout << "FLoating point:";
	for (int i=0;i<32;i++){
		if(i%4==0)
			cout << endl;
		cout << *(float*)(&SRegisters->floatingPointRegisters[i])<<"\t";
	}
	cout << endl << endl;
	cout<<"ASR: ";
	for (int i=0;i<32;i++){
	cout << SRegisters->asrRegisters[i] <<" ";
	}
	cout << endl;
	cout<<"CPR: ";
	for (int i=0;i<32;i++){
	cout << SRegisters->CoprocessorRegisters[i] <<" ";
	}
	cout << endl;


	cout <<"WIM:";disp_int(SRegisters->wim,31,0);
	cout<<"Y:  ";disp_int(SRegisters->y,31,0);
	cout<<"PSR:";disp_int(SRegisters->psr->getPSR(),31,0);
	cout<<"S:  "<<SRegisters->psr->getS()<<endl;
	cout<<"CWP:"<<SRegisters->psr->getCwp()<<endl;
	cout<<"ET: "<<SRegisters->psr->getEt()<<endl;
	cout<<"PS: "<<SRegisters->psr->getPs()<<endl;
	cout<<"C:  "<<SRegisters->psr->getC()<<endl;
	cout<<"N:  "<<SRegisters->psr->getN()<<endl;
	cout<<"V:  "<<SRegisters->psr->getV()<<endl;
	cout<<"Z:  "<<SRegisters->psr->getZ()<<endl;
	cout<<"FSR:";disp_int(SRegisters->fsr->getFSR(),31,0);
	cout<<"TBR:";disp_int(SRegisters->tbr->getTbr(),31,0);
	cout << endl;

	cout<<"PC: "<<SRegisters->pc<<"\n";
	cout<<"Next PC: "<<SRegisters->npc<<"\n";

	cout << "\n[END] register file dump" << dec << endl;
}

registerfile::~registerfile()
{
	delete SRegisters;
}

std::string* registerfile::getStatistics()
{
	return 0;
}

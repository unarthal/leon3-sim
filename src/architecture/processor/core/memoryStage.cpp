#include "architecture/processor/core/memoryStage.h"
#include "architecture/processor/core/core.h"
#include <iostream>
#include "architecture/processor/core/executeStage.h"
#include "architecture/system.h"
#include "generic/utility.h"
#include <climits>
#include "architecture/memory/memory.h"
#include "architecture/constants_typedefs.h"
#include "architecture/memory/simple_memory.h" //TODO memoryStage should be oblivious of the type of memory being employed
#include "architecture/processor/processor.h"
#include "architecture/processor/cache/cache.h"

using namespace std;

memoryStage::memoryStage(core* x_containingCore) : element()
{
	m_containingCore = x_containingCore;
	m_executeStage_memoryStage_interface = 0;
	m_memoryStage_exceptionStage_interface = 0;
	m_exceptionStage = m_containingCore->getExceptionStage();
	m_waitingForMemoryResponse = false;
	m_hasTrapOccurred = false;

	pmc_loads = 0;
	pmc_stores = 0;
}

interface* memoryStage::getMemoryStageDcacheInterface()
{
	return m_memoryStage_dcache_interface;
}
void memoryStage::setMemoryStageDcacheInterface(interface* x_memoryStage_dcache_interface)
{
	m_memoryStage_dcache_interface = x_memoryStage_dcache_interface;
}
interface* memoryStage::getDcacheMemoryStageInterface()
{
	return m_dcache_memoryStage_interface;
}
void memoryStage::setDcacheMemoryStageInterface(interface* x_dcache_memoryStage_interface)
{
	m_dcache_memoryStage_interface =x_dcache_memoryStage_interface;
}

memoryStage::~memoryStage()
{

}

void xc_copy(executeStagememoryStageMessage* m_msg, memoryStageexceptionStageMessage* x_msg);
void rw_copy(executeStagememoryStageMessage* m_msg, memoryStageexceptionStageMessage* x_msg);

void memoryStage::simulateOneCycle()
{
	processMessageFromMemory();

	processMessageFromExecuteStage();

	if((m_memoryStage_exceptionStage_interface->getBusy() == true
			&& m_executeStage_memoryStage_interface->doesElementHaveAnyPendingMessage(this) == true)
			|| m_waitingForMemoryResponse == true)
	{
		m_executeStage_memoryStage_interface->setBusy(true);
	}
	else
	{
		m_executeStage_memoryStage_interface->setBusy(false);
	}
}

void memoryStage::processMessageFromMemory()
{
	if(m_dcache_memoryStage_interface->doesElementHaveAnyPendingMessage(this) == true && m_hasTrapOccurred == true)
	{
		//assuming there can be only one pending message
		delete (memorymessage*) (m_dcache_memoryStage_interface->popElementsPendingMessage(this));
		m_waitingForMemoryResponse = false;
		return;
	}

	if(m_dcache_memoryStage_interface->doesElementHaveAnyPendingMessage(this) == true
			&& m_memoryStage_exceptionStage_interface->getBusy() == false)
	{
		if(m_executeStage_memoryStage_interface->doesElementHaveAnyPendingMessage(this) == false)
		{
			showErrorAndExit("[MEMORY stage] received a memory response for an unknown request");
		}

		executeStagememoryStageMessage* m_msg = (executeStagememoryStageMessage*) (m_executeStage_memoryStage_interface->popElementsPendingMessage(this));

		if(m_msg->isMemIns == false)
		{
			showErrorAndExit("[MEMORY stage] received a memory response for an unknown request");
		}
		instruction* m_inst = m_msg->getMInst();

		memorymessage* mem_msg = (memorymessage*) (m_dcache_memoryStage_interface->popElementsPendingMessage(this));
		//std::cout << dec << "[" << getClock() << "] M RESP : " << mem_msg->getMemoryMessageType() << " : " << hex << mem_msg->getAddress() << dec << "\n";

		memoryStageexceptionStageMessage* x_msg = new memoryStageexceptionStageMessage(this, m_exceptionStage, m_inst, m_msg->getMPC());
		x_msg->retVal = m_msg->retVal;
		xc_copy(m_msg, x_msg);
		rw_copy(m_msg, x_msg);
		int returnValue = RET_SUCCESS;

		char* byteArray;
		addrType memoryAddress = mem_msg->getAddress();
		if(memoryAddress != m_msg->m_addr) //memory response should be for the request that is in the ex-me latch
		{
			showErrorAndExit("[MEMORY stage] received a memory response for an unknown request");
		}

		if (mem_msg->getMemoryMessageType() == ReadResponse)
		{
			byteArray = mem_msg->getValue();
		}

		//note that no endian conversions are performed. values obtained in the big-endian forms
		//are forwarded to subsequent stages and to the register file.
	
		if (m_msg->isLoad == true && m_msg->isInt == true )///LoadIntegerInstructions
		{
			switch(m_inst->op3)
			{
			case 9: //LDSB opcode:001001
				x_msg->rw_RD = m_inst->rd;
				x_msg->rw_RD_val = signExtendByteToWord(convertBigEndianByteArrayToLittleEndianInteger(byteArray, 1));
				x_msg->rw_isIntReg = true;
				break;

			case 10: //LDSH opcode:001010
				if(is_mem_address_not_aligned(memoryAddress, HALFWORD_ALIGN))
				{
					cout << ("Source memory address not half word aligned") << endl;
					returnValue = mem_address_not_aligned;
				}
				x_msg->rw_RD = m_inst->rd;
				x_msg->rw_RD_val = signExtendHalfWordToWord(convertBigEndianByteArrayToLittleEndianInteger(byteArray, 2));
				x_msg->rw_isIntReg = true;
				break;

			case 1: //LDUB opcode:000001
			case 2: //LDUH opcode:000010
			case 0: //LD   opcode:000000
				x_msg->rw_RD = m_inst->rd;
				x_msg->rw_RD_val = convertBigEndianByteArrayToLittleEndianInteger(byteArray, 4);

				x_msg->rw_isIntReg = true;
				break;

			case 3: //LDD opcode:000011
				if(is_register_mis_aligned(m_inst->rd))
				{
					cout << ("LDD: destination register not word aligned") << endl;
				}
				x_msg->rw_RD = m_inst->rd;
				x_msg->rw_RD_val = convertBigEndianByteArrayToLittleEndianInteger(byteArray, 4);
				x_msg->rw_nextRD = m_inst->rd+1;
				x_msg->rw_nextRD_val = convertBigEndianByteArrayToLittleEndianInteger(byteArray+4, 4);

				x_msg->rw_isIntReg = true;
				break;
			}

			//cout << "loaded value = " << x_msg->rw_RD_val << "\n";
			delete [] (byteArray);
			returnValue = RET_SUCCESS;
		}

		else if(m_msg->isLoad == true && m_msg->isFloat == true ) ///LoadFloatingPointInstructions
		{
			switch(m_inst->op3)
			{
			case 32: //LDF  opcode: 100000
				x_msg->rw_F_nWords = 1;
				x_msg->rw_F_RD1 = m_inst->rd;
				x_msg->rw_F_RD1_val = convertBigEndianByteArrayToLittleEndianInteger(byteArray, 4);
				x_msg->rw_isFltReg = true;
				break;

			case 35: //LDDF opcode: 100011
				if(is_register_mis_aligned(m_inst->rd))
				{
					// An integer load instruction
					cout << ("Destination is an odd-even register pair") <<endl;
					int t_fsr = m_msg->FSR;
					t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
					m_msg->rw_FSR = t_fsr;
					m_msg->rw_isFSR = true;
					returnValue = fp_exception;
				}

				//TODO LDDD not handled
				break;

			case 33: //LDFSR opcode:100001
				x_msg->rw_FSR = convertBigEndianByteArrayToLittleEndianInteger(byteArray, 4);
				x_msg->rw_isFSR = true;
				break;
			}

			delete [] (byteArray);
			returnValue = RET_SUCCESS;
		}

		else if(m_inst->op == 37) //STFSR
		{
			int t_fsr = m_msg->FSR;
			t_fsr = modifyBits(0, t_fsr, FSR_ftt_l, FSR_ftt_r);
			x_msg->rw_FSR = t_fsr;
			x_msg->rw_isFSR = true;
		}

		else if(m_msg->isAtomic == true)
		{
			//// Atomic Operations
			char byte = byteArray[0];
			if(m_inst->op3==13)
			{
				//LDSTUB OP3=001101
				regType regRD = ((unsigned int)(((unsigned int)byte)<<24)>>24);
				x_msg->rw_RD = m_inst->rd;
				x_msg->rw_RD_val = regRD;
				x_msg->rw_isIntReg = true;
				returnValue = RET_SUCCESS;
			}
			else if(m_inst->op3==29)
			{
				//LDSTUBA OP3=011101
				cout << "LDSTUBA Trap, No Alternate space" << endl;
				returnValue = privileged_instruction;
				returnValue = RET_QUIT;
			}
		}

		else if(m_msg->isSwap == true)
		{
			//// SWAP Operations
			if(m_inst->op3==15)
			{
				x_msg->rw_RD = m_inst->rd;
				x_msg->rw_RD_val = convertBigEndianByteArrayToLittleEndianInteger(byteArray, 4);
				x_msg->rw_isIntReg = true;
			}
			returnValue = RET_SUCCESS;
		}

		if(m_msg->isMemIns == true && m_msg->retVal != RET_SUCCESS)
		{
			x_msg->retVal = returnValue;
		}

		m_memoryStage_exceptionStage_interface->addPendingMessage(x_msg);
		delete mem_msg; //remember to delete messages and events once their work is done!
		delete m_msg;
		m_waitingForMemoryResponse = false;
	}
}

void memoryStage::processMessageFromExecuteStage()
{
	if(m_executeStage_memoryStage_interface->doesElementHaveAnyPendingMessage(this) == true && m_hasTrapOccurred == true)
	{
		executeStagememoryStageMessage* m_msg = (executeStagememoryStageMessage*) (m_executeStage_memoryStage_interface->peekElementsPendingMessage(this)); //note we peek here. busy is set to true if it is a memory instruction. we pop when the response arrives. busy is set to false then.
		addrType m_pc = m_msg->getMPC();
		if(m_pc == m_containingCore->getExceptionStage()->getNewPCAfterTrap())
		{
			m_hasTrapOccurred = false;
		}
		else
		{
			//assuming there can be only one pending message
			m_executeStage_memoryStage_interface->popElementsPendingMessage(this);
			delete m_msg;
			m_waitingForMemoryResponse = false;
			return;
		}
	}

	if(m_executeStage_memoryStage_interface->doesElementHaveAnyPendingMessage(this) == true
				&& m_memoryStage_exceptionStage_interface->getBusy() == false
				&& m_waitingForMemoryResponse == false)
	{
		executeStagememoryStageMessage* m_msg = (executeStagememoryStageMessage*) (m_executeStage_memoryStage_interface->peekElementsPendingMessage(this)); //note we peek here. busy is set to true if it is a memory instruction. we pop when the response arrives. busy is set to false then.
		instruction* m_inst = m_msg->getMInst();
		addrType m_pc = m_msg->getMPC();
		//std::cout << dec << "[" << getClock() << "] M RECV : " << hex << m_pc << " : " << m_inst->instructionWord << dec << "\n";
		
		int returnValue = RET_SUCCESS;

		if(m_msg->isMemIns == true)
		{
			addrType memoryAddress = m_msg->m_addr;
			if(m_msg->isInt == true &&  m_msg->isLoad == true)
			{
				memorymessage* msg = 0;
				switch(m_inst->op3)
				{
				case 9:
				case 1:
					{
						msg = new memorymessage(this, m_containingCore->getDcache(), Read, memoryAddress, 1, 0);
						break;
					}
				case 10:
				case 2:
					{
						if (is_mem_address_not_aligned(memoryAddress, HALFWORD_ALIGN))
						{
							cout << ("memory address not half word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}
						msg = new memorymessage(this, m_containingCore->getDcache(), Read, memoryAddress, 2, 0);
						break;
					}
				case 0:
					{
						if (is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
						{
							cout << ("memory address not word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}
						msg = new memorymessage(this, m_containingCore->getDcache(), Read, memoryAddress, 4, 0);
						break;
					}
				case 3:
					{
						if (is_mem_address_not_aligned(memoryAddress, DOUBLEWORD_ALIGN))
						{
							cout << ("memory address not word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}
						msg = new memorymessage(this, m_containingCore->getDcache(), Read, memoryAddress, 8, 0);//TODO to be split into 2 accesses of 4 bytes each
						break;
					}
				default: showErrorAndExit("[MEMORY stage] unknown integer load instruction");
				//TODO load from alternate space instructions not implemented
				}

				m_memoryStage_dcache_interface->addPendingMessage(msg);
				pmc_loads++;
			}

			else if(m_msg->isFloat == true && m_msg->isLoad == true)
			{
				//TODO float loads
			}

			else if (m_msg->isInt == true && m_msg->isStore == true) ///StoreIntegerInstructions
			{
				regType regRD = m_msg->regRD;
				regType regNextRD = m_msg->regNextRD;

				memorymessage* msg = 0;
				switch(m_inst->op3)
				{
				case 5:
					{
						char byte = regRD & 0x000000FF;
						msg = new memorymessage(this, m_containingCore->getDcache(), Write, memoryAddress, 1, convertLittleEndianIntegerToBigEndianByteArray(byte, 1));
						break;
					}
				case 6:
					{
						if (is_mem_address_not_aligned(memoryAddress, HALFWORD_ALIGN))
						{
							cout << ("memory address not word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}
						short halfWord = regRD & 0x0000FFFF;
						msg = new memorymessage(this, m_containingCore->getDcache(), Write, memoryAddress, 2, convertLittleEndianIntegerToBigEndianByteArray(halfWord, 2));
						break;
					}
				case 4:
					{
						if (is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
						{
							cout << ("memory address not word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}
						msg = new memorymessage(this, m_containingCore->getDcache(), Write, memoryAddress, 4, convertLittleEndianIntegerToBigEndianByteArray(regRD, 4));
						//cout << hex << "storing value " << regRD << " at " << memoryAddress << dec << endl;
						break;
					}
				case 7:
					{
						if (is_mem_address_not_aligned(memoryAddress, DOUBLEWORD_ALIGN))
						{
							cout << ("Destination memory address not word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}
						if(is_register_mis_aligned(m_inst->rd))
						{
							cout << ("STD: destination register not word aligned") << endl;
						}
						char *val = new char[8];
						char *tmp = convertLittleEndianIntegerToBigEndianByteArray(regRD, 4);
						for(int i = 0; i < 4; i++)
						{
							val[i] = tmp[i];
						}
						delete tmp;
						tmp = convertLittleEndianIntegerToBigEndianByteArray(regNextRD, 4);
						for(int i = 0; i < 4; i++)
						{
							val[i+4] = tmp[i];
						}
						delete tmp;
						msg = new memorymessage(this, m_containingCore->getDcache(), Write, memoryAddress, 8, val);//TODO to be split into 2 accesses of 4 bytes each
						break;
					}
				default: showErrorAndExit("[MEMORY stage] unknown integer store instruction");
				//TODO load from alternate space instructions not implemented
				}

				m_memoryStage_dcache_interface->addPendingMessage(msg);
				pmc_stores++;
			}

			else if(m_msg->isFloat == true && m_msg->isStore == true) ///StoreFloatInstructions
			{
				regType regRD = m_msg->regRD;
				regType regNextRD = m_msg->regNextRD;

				memorymessage* msg = 0;
				switch(m_inst->op)
				{
				case 36: //STF
					{
						if(is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
						{
							cout << ("Source memory address double word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}

						msg = new memorymessage(this, m_containingCore->getDcache(), Write, memoryAddress, 4, convertLittleEndianIntegerToBigEndianByteArray(regRD, 4));
						break;
					}
				case 39: //STDF
					{
						if(is_register_mis_aligned(m_inst->rd))
						{
							//TODO check exception cases -- detection and handling -- everywhere
							cout << ("FP exception: Source is an odd-even register pair") << endl;
							int t_fsr = m_msg->FSR;
							t_fsr = modifyBits(INVALID_FP_REGISTER, t_fsr, FSR_ftt_l, FSR_ftt_r);
							memoryStageexceptionStageMessage* x_msg = new memoryStageexceptionStageMessage(this, m_exceptionStage, m_inst, m_msg->getMPC());
							x_msg->rw_FSR = t_fsr;
							x_msg->rw_isFSR = true;
							x_msg->retVal = fp_exception;
							xc_copy(m_msg, x_msg);
							rw_copy(m_msg, x_msg);
							m_memoryStage_exceptionStage_interface->addPendingMessage(x_msg);
							m_executeStage_memoryStage_interface->setBusy(false);
							delete m_msg;
							return;
						}

						if(is_mem_address_not_aligned(memoryAddress, DOUBLEWORD_ALIGN))
						{
							cout << ("Source memory address double word aligned") << endl;
							returnValue = mem_address_not_aligned;
						}

						//TODO largestValueType val = regRD | (largestValueType)regNextRD << 32;
						//TODO msg = new memorymessage(this, m_containingCore->getDcache(), Write, memoryAddress, 8, convertLittleEndianIntegerToBigEndianByteArray(val, 8));
						break;
					}
				case 38: //STDFQ
					{
						cout << "FP EXCEPTION TRAP" << endl;
						int t_fsr = m_msg->FSR;
						t_fsr = modifyBits(4, t_fsr, FSR_ftt_l, FSR_ftt_r);
						memoryStageexceptionStageMessage* x_msg = new memoryStageexceptionStageMessage(this, m_exceptionStage, m_inst, m_msg->getMPC());
						x_msg->rw_FSR = t_fsr;
						x_msg->rw_isFSR = true;
						x_msg->retVal = fp_exception;
						xc_copy(m_msg, x_msg);
						rw_copy(m_msg, x_msg);
						m_memoryStage_exceptionStage_interface->addPendingMessage(x_msg);
						m_executeStage_memoryStage_interface->setBusy(false);
						delete m_msg;
						return;
					}
				case 37: //STFSR
					{
						int fsr = m_msg->FSR;
						msg = new memorymessage(this, m_containingCore->getDcache(), Write, memoryAddress, 4, convertLittleEndianIntegerToBigEndianByteArray(fsr, 4));
						break;
					}
				}

				if(msg != 0)
				{
					m_memoryStage_dcache_interface->addPendingMessage(msg);
				}
			}
			/* TODO atomic operations
			else if(m_msg->isAtomic == true)   //// Atomic Operations
			{
				memoryMessage newMessage1 (this, MemoryRead,
						m_msg->m_addr, 0,
						0, 0, 1,
						clock_cycles+MEM_LAT, 40);

				MEM_Interface->pendingMemoryAccessRequests.push(newMessage1);
				m_msg->nReq += 1;

				if(ins->op3==13)
				{
					//LDSTUB OP3=001101
					memoryMessage newMessage (this, MemoryWrite,
							m_msg->m_addr, 0xFF,
							0, 0, 1,
							clock_cycles+MEM_LAT, 441);

					MEM_Interface->pendingMemoryAccessRequests.push(newMessage);
					m_msg->nReq += 1;
					returnValue = RET_SUCCESS;
				}
				// else if(ins->op3==29){
				//     //LDSTUBA OP3=011101
				//     xout << "LDSTUBA Trap, No Alternate space" << endl;
				//     returnValue = privileged_instruction;
				// }

				returnValue = RET_QUIT;
			}
			else if (m_msg->isSwap == true)//TODO swap instruction
			{
				//// SWAP Operations
				regType regRD = m_msg->regRD;
				if(ins->op3==15)
				{
					//SWAP opcode:001111
					if (is_mem_address_not_aligned(memoryAddress, WORD_ALIGN))
					{
						xout << ("Destination memory address not word aligned") << endl;
						returnValue = mem_address_not_aligned;
					}

					memoryMessage newMessage1 (this, MemoryRead,
							m_msg->m_addr, 0,
							0, 0, 8,
							clock_cycles+MEM_LAT, 43);

					MEM_Interface->pendingMemoryAccessRequests.push(newMessage1);
					m_msg->nReq += 1;

					memoryMessage newMessage2 (this, MemoryWrite,
							m_msg->m_addr, 0,
							0, regRD, 8,
							clock_cycles+MEM_LAT, 441);

					MEM_Interface->pendingMemoryAccessRequests.push(newMessage2);
					m_msg->nReq += 1;
				}

				returnValue = RET_SUCCESS;
			}*/
			// else
			// {
			//      /// until all RW complete,
			//      //  Use :to check that #Cycles are multiples
			//      //  of original 0 latency cycles (without interlocks)
			//
			//      xout<<"dummy\n";
			//      memoryMessage dummy (this, MemoryRead,
			//              m_msg->m_addr+4, 0,
			//              0, 0, 4,
			//              clock_cycles+MEM_LAT, 432);
			//
			//      MEM_Interface->pendingMemoryAccessRequests.push(dummy);
			//      m_msg->countReq=0;
			//      m_msg->nReq = 1;
			//  }
			if(returnValue != RET_SUCCESS)
			{
				memoryStageexceptionStageMessage* x_msg = new memoryStageexceptionStageMessage(this, m_exceptionStage, m_msg->getMInst(), m_msg->getMPC());
				x_msg->retVal = returnValue;
				xc_copy(m_msg, x_msg);
				rw_copy(m_msg, x_msg);
				m_memoryStage_exceptionStage_interface->addPendingMessage(x_msg);
				m_waitingForMemoryResponse = false;
				delete m_msg;
			}
			else
			{
				m_waitingForMemoryResponse = true;
			}
		}
		else
		{
			//not a memory operation
			executeStagememoryStageMessage* m_msg = (executeStagememoryStageMessage*) (m_executeStage_memoryStage_interface->popElementsPendingMessage(this));
			memoryStageexceptionStageMessage* x_msg = new memoryStageexceptionStageMessage(this, m_exceptionStage, m_msg->getMInst(), m_msg->getMPC());
			x_msg->retVal = m_msg->retVal;
			xc_copy(m_msg, x_msg);
			rw_copy(m_msg, x_msg);
			m_memoryStage_exceptionStage_interface->addPendingMessage(x_msg);
			m_waitingForMemoryResponse = false;
			delete m_msg;
		}
	}
}

void memoryStage::setHasTrapOccurred()
{
	m_hasTrapOccurred = true;
}

std::string* memoryStage::getStatistics()
{
	return 0;
}

core* memoryStage::getContainingCore()
{
	return m_containingCore;
}

counterType memoryStage::getNumberOfLoads()
{
	return pmc_loads;
}

counterType memoryStage::getNumberOfStores()
{
	return pmc_stores;
}

void memoryStage::setExecuteMemoryInterface(interface* x_executeStage_memoryStage_interface)
{
	m_executeStage_memoryStage_interface = x_executeStage_memoryStage_interface;
}

void memoryStage::setMemoryExceptionInterface(interface* x_memoryStage_exceptionStage_interface)
{
	m_memoryStage_exceptionStage_interface = x_memoryStage_exceptionStage_interface;
}

memoryStageexceptionStageMessage::memoryStageexceptionStageMessage(element* x_producer, element* x_consumer, instruction* x_x_inst, unsigned int x_x_pc) : message(x_producer, x_consumer)
{
	m_x_inst = x_x_inst;
	m_x_pc = x_x_pc;

	retVal = 0;

	//for the XC stage
	xc_isError = false;
	xc_psr = 0;
	xc_tbr = 0;
	xc_nWP = 0;
	xc_PC = 0;
	xc_nPC = 0;

	//for the RW stage
	rw_isIntReg = false;
	rw_isFltReg = false;
	rw_isASR = false;
	rw_isPSR = false;
	rw_isFSR = false;
	rw_isTBR = false;
	rw_isY = false;
	rw_isWIM = false;
	rw_isControl = false;
	rw_isTrapSaveCounter = false;
	    /////////////////////////////
	rw_RD = 0;
	rw_RD_val = 0;
	rw_F_nWords = 0;
	rw_F_RD1 = 0;
	rw_F_RD2 = 0;
	rw_F_RD3 = 0;
	rw_F_RD4 = 0;
	rw_F_RD1_val = 0;
	rw_F_RD2_val = 0;
	rw_F_RD3_val = 0;
	rw_F_RD4_val = 0;
	rw_PC = 0;
	rw_nPC = 0;
	rw_PSR = 0;
	rw_FSR = 0;
	rw_TBR = 0;
	rw_Y = 0;
	rw_WIM = 0;
}

memoryStageexceptionStageMessage::~memoryStageexceptionStageMessage()
{

}

instruction* memoryStageexceptionStageMessage::getXInst()
{
	return m_x_inst;
}

unsigned int memoryStageexceptionStageMessage::getXPC()
{
	return m_x_pc;
}

void xc_copy(executeStagememoryStageMessage* m_msg, memoryStageexceptionStageMessage* x_msg)
{
	x_msg->xc_isError = m_msg->xc_isError;
	x_msg->xc_psr = m_msg->xc_psr;
	x_msg->xc_tbr = m_msg->xc_tbr;
	x_msg->xc_nWP = m_msg->xc_nWP;
	x_msg->xc_PC = m_msg->xc_PC;
	x_msg->xc_nPC = m_msg->xc_nPC;
}

void rw_copy(executeStagememoryStageMessage* m_msg, memoryStageexceptionStageMessage* x_msg)
{
	x_msg->rw_isIntReg = m_msg->rw_isIntReg;
	x_msg->rw_isFltReg = m_msg->rw_isFltReg;
	x_msg->rw_isASR = m_msg->rw_isASR;
	x_msg->rw_isPSR = m_msg->rw_isPSR;
	x_msg->rw_isFSR = m_msg->rw_isFSR;
	x_msg->rw_isTBR = m_msg->rw_isTBR;
	x_msg->rw_isY = m_msg->rw_isY;
	x_msg->rw_isWIM = m_msg->rw_isWIM;
	x_msg->rw_isControl = m_msg->rw_isControl;
	x_msg->rw_isTrapSaveCounter = m_msg->rw_isTrapSaveCounter;
		    /////////////////////////////
	x_msg->rw_RD = m_msg->rw_RD;
	x_msg->rw_RD_val = m_msg->rw_RD_val;
	x_msg->rw_F_nWords = m_msg->rw_F_nWords;
	x_msg->rw_F_RD1 = m_msg->rw_F_RD1;
	x_msg->rw_F_RD2 = m_msg->rw_F_RD2;
	x_msg->rw_F_RD3 = m_msg->rw_F_RD3;
	x_msg->rw_F_RD4 = m_msg->rw_F_RD4;
	x_msg->rw_F_RD1_val = m_msg->rw_F_RD1_val;
	x_msg->rw_F_RD2_val = m_msg->rw_F_RD2_val;
	x_msg->rw_F_RD3_val = m_msg->rw_F_RD3_val;
	x_msg->rw_F_RD4_val = m_msg->rw_F_RD4_val;
	x_msg->rw_PC = m_msg->rw_PC;
	x_msg->rw_nPC = m_msg->rw_nPC;
	x_msg->rw_PSR = m_msg->rw_PSR;
	x_msg->rw_FSR = m_msg->rw_FSR;
	x_msg->rw_TBR = m_msg->rw_TBR;
	x_msg->rw_Y = m_msg->rw_Y;
	x_msg->rw_WIM = m_msg->rw_WIM;
}

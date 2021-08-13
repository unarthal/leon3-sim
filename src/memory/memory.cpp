#include "memory.h"
#include "generic/header.h"
#include "processor/constants.h"
#include "memory_checks.h"

Memory::Memory()
{
    memory=new char[MAX_MEM];
}

void Memory::perform()
{
}

int Memory::setByte(unsigned long memoryAddress, char byte)
{
     memory[memoryAddress] = byte;
}

char Memory::getByte(unsigned long memoryAddress)
{
    return memory[memoryAddress];
}

void Memory::handleEvent ()  {
    // xout<<"handling Memory Request\n";
    MEM_Interface->reqID = MEM_Interface->pendingMemoryAccessRequests.top().reqID; //// writng on interfaces' reqID line for particular client to discriminate between requests

    if(MEM_Interface->pendingMemoryAccessRequests.top().eventType == MemoryRead){
        // xout<<"MemoryRead\n";
        switch (MEM_Interface->pendingMemoryAccessRequests.top().size)
        {
        case 1:
            MEM_Interface->read_byte = Memory::getByte(MEM_Interface->pendingMemoryAccessRequests.top().addr);
            break;
        case 4:
            MEM_Interface->read_halfword = readWordAsString(MEM_Interface->pendingMemoryAccessRequests.top().addr, *this);
            break;
        case 8:
            MEM_Interface->read_word = Memory::getWord(MEM_Interface->pendingMemoryAccessRequests.top().addr);
            break;
        default:
            xout<<"MEM_INTERFACE: INVALID MEMORY READ REQ TYPE\n";
            exit(0);
            break;
        }
        nMR++;

    } else if(MEM_Interface->pendingMemoryAccessRequests.top().eventType == MemoryWrite){
        // xout<<"MemoryWrite\n";
        switch (MEM_Interface->pendingMemoryAccessRequests.top().size)
        {
            case 1:
                MEM_Interface->write_status = this->setByte(MEM_Interface->pendingMemoryAccessRequests.top().addr, MEM_Interface->pendingMemoryAccessRequests.top().write_byte);
                break;
            case 4:
                MEM_Interface->write_status = this->writeHalfWord(MEM_Interface->pendingMemoryAccessRequests.top().addr, MEM_Interface->pendingMemoryAccessRequests.top().write_halfword);
                break;
            case 8:
                MEM_Interface->write_status = this->setWord(MEM_Interface->pendingMemoryAccessRequests.top().addr, MEM_Interface->pendingMemoryAccessRequests.top().write_word);
                break;
            default:
                xout<<"MEM_INTERFACE: INVALID MEMORY WRITE REQ TYPE\n";
                break;
        }
        nMW++;
    }

}

/*
 * Returns the word located at <memoryAddress> in memory.
 */
unsigned long Memory::getWord(unsigned long memoryAddress)
{        
    unsigned long word, hexDigit;
    
    /* Reads four bytes one by one starting from lowest to highest. Once a byte is read, it is left shifted
     * by 24 bits followed by right shifted by 24 bits to clear higher order 24 bits, if set by sign extension 
     * caused by widening of data during auto-casting. Casting takes place because of hexDigit being an
     * unsigned long (32 bits) while getByte() returns data of type char (8 bits). All four bytes are
     * packed together to form a 32 bit word.
     */
    hexDigit = getByte(memoryAddress); memoryAddress++; hexDigit = (hexDigit << 24) >> 24; word = hexDigit;
    hexDigit = getByte(memoryAddress); memoryAddress++; hexDigit = (hexDigit << 24) >> 24; word = (word << 8) | hexDigit;
    hexDigit = getByte(memoryAddress); memoryAddress++; hexDigit = (hexDigit << 24) >> 24; word = (word << 8) | hexDigit;
    hexDigit = getByte(memoryAddress); memoryAddress++; hexDigit = (hexDigit << 24) >> 24; word = (word << 8) | hexDigit;
    
    return word;
}

int Memory::setWord(unsigned long memoryAddress, unsigned long word)
{ 
    //xout << "set word: "<<memoryAddress <<endl;
    char byte;
    byte = (word & 0xFF000000) >> 24;               // Write the first byte.
    setByte(memoryAddress++, byte);
    byte = (word & 0x00FF0000) >> 16;               // Write the second byte.
    setByte(memoryAddress++, byte);
    byte = (word & 0x0000FF00) >> 8;                // Write the third byte.
    setByte(memoryAddress++, byte);
    byte = word & 0x000000FF;                       // Write the fourth byte.
    setByte(memoryAddress, byte);
    //xout << " AS: "<< word << endl;
	return RET_SUCCESS;
}


/*
 * Writes the Half Word located at <memoryAddress> in memory with <halfWord>.
 */
int Memory::writeHalfWord(unsigned long memoryAddress, unsigned short halfWord)
{
    char byte;
    byte = (halfWord & 0xFF00) >> 8;            // Write the first byte.
    setByte(memoryAddress++, byte);
    byte = halfWord & 0x00FF;                   // Write the second byte.
    setByte(memoryAddress, byte);
        
	return RET_SUCCESS;
}

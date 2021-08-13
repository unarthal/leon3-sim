#ifndef MEMORY_H
#define MEMORY_H

#include "interface/eheader.h"


class Memory : public Element 
{
    private:
        char *memory;

    public:
        memoryInterface* MEM_Interface;   
        Memory();
        
        
        int setByte(unsigned long memoryAddress, char byte);
        char getByte(unsigned long memoryAddress);
        
        unsigned long getWord(unsigned long memoryAddress);
        int setWord(unsigned long memoryAddress, unsigned long word);
        

        int writeHalfWord(unsigned long memoryAddress, unsigned short halfWord);

        void handleEvent();
        void perform();
        
};

#endif

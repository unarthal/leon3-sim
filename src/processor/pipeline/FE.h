#ifndef FE_H
#define FE_H

#include "FE_DE_latch.h"
#include "RW_FE_latch.h"
#include "interface/Element.h"
#include "interface/memoryInterface.h"
#include "processor/registers/register.h"
#include "interface/Event.h"

extern int MEM_LAT;

class FE : public Element 
{
    public:
        Register* sregister;
        FE_DE_latch* fede_latch;
        RW_FE_latch* rwfe_latch;
        memoryInterface* MEM_Interface;

        FE(Register* reg, FE_DE_latch* fede, RW_FE_latch* rwfe);
        void perform();
        void handleEvent();
};

#endif

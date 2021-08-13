#ifndef ME_H
#define ME_H

#include "ME_XC_latch.h"
#include "EX_ME_latch.h"
#include "interface/Element.h"
#include "interface/memoryInterface.h"

class ME : Element {
public:
    EX_ME_latch* exme_latch;
    ME_XC_latch* mexc_latch;
    memoryInterface* MEM_Interface;

public:
    ME( EX_ME_latch* exme, ME_XC_latch* mexc );
    void perform();
    void handleEvent();

};

#endif

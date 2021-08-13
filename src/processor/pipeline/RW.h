#ifndef RW_H
#define RW_H

#include <strings.h>
#include <iostream>
#include "RW_FE_latch.h"
#include "XC_RW_latch.h"
#include "interface/Element.h"
#include "processor/registers/register.h"

class RW : Element 
{
        Register* sregister;
        XC_RW_latch* xcrw_latch;
        RW_FE_latch* rwfe_latch;

    public:
        RW(Register* reg, XC_RW_latch* xcrw, RW_FE_latch* rwfe);
        void perform();
        void handleEvent();
};

#endif

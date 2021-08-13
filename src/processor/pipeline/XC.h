#ifndef XC_H
#define XC_H

#include "XC_RW_latch.h"
#include "ME_XC_latch.h"
#include "interface/Element.h"

class XC : public Element 
{
        ME_XC_latch* mexc_latch;
        XC_RW_latch* xcrw_latch;
    public:
        XC( ME_XC_latch* mexc, XC_RW_latch* xcrw);
        int trap(int memAddr);
        void perform();
        void handleEvent();
};

#endif

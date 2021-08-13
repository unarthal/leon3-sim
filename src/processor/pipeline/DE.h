#ifndef DE_H
#define DE_H

#include "DE_RA_latch.h"
#include "FE_DE_latch.h"
#include "interface/Element.h"

using namespace std;

class DE : public Element 
{
        FE_DE_latch* fede_latch;
        DE_RA_latch* dera_latch;
    public:
        DE(FE_DE_latch* fede, DE_RA_latch* dera);
        ~DE();
        void perform ();
        void handleEvent();
};

#endif

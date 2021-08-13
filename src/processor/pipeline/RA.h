#ifndef RA_H
#define RA_H

#include "RA_EX_latch.h"
#include "DE_RA_latch.h"
#include "interface/Element.h"
#include "processor/registers/register.h"

class RA : public Element {
    public:
        Register* sregister;
        RA_EX_latch* raex_latch;
        DE_RA_latch* dera_latch;
        RA(Register* reg, RA_EX_latch* raex, DE_RA_latch* dera);
        void perform ();
        void handleEvent ();
};

#endif

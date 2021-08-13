#include <strings.h>
#include "generic/utility.h"
#include "FE_DE_latch.h"
#include "RW_FE_latch.h"
#include "FE.h"
#include "memory.h"
#include "processor/constants.h"
#include "interface/memoryInterface.h"

extern int MEM_LAT;

FE::FE( Register* reg, FE_DE_latch* fede, RW_FE_latch* rwfe)
{
    MEM_Interface = NULL;
    sregister =  reg;
    fede_latch = fede;
    rwfe_latch = rwfe;
}

void FE::perform()
{
    if (rwfe_latch->isFE_Enabled == false)
    {
        xout<<"FE STALL\n"; 
        return;
    }
    else
    {
        xout<<"FE start\n";
    }

    memoryMessage newMessage(this, MemoryRead, 
            sregister->getPC(), 0, 
            0, 0, 
            4, clock_cycles+MEM_LAT,
            0 );

    MEM_Interface->pendingMemoryAccessRequests.push(newMessage);

    rwfe_latch->isFE_Enabled = false;
}

void FE::handleEvent()
{
    fede_latch->Ins = MEM_Interface->read_halfword;
    fede_latch->isDE_Enabled = true;
    nINS++;

}

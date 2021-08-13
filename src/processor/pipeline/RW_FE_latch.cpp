#include "RW_FE_latch.h"
#include <strings.h>

RW_FE_latch::RW_FE_latch()
{
    isFE_Enabled = true;
    bzero(this, sizeof(*this));
}


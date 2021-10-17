#include "architecture/processor/core/registers/tbr.h"
void trap_base_register::setTbr(int setVal){
    tbr=setVal;
}
int trap_base_register::getTbr(void){
     return tbr;
}
int trap_base_register::getTba(void){
    int tba = tbr;
    tba = (((unsigned int)tba)>>12);
    return tba;
}
void trap_base_register::setTba(int setVal){
    //setVal = setVal << 12;
    tbr = tbr & 0x00000FFF;
    tbr = tbr | setVal;
}
int trap_base_register::getTt(void){
    int tt = ((unsigned int)(tbr<<20)>>24);
    return tt;

}
void trap_base_register::setTt(int setVal){
    setVal = setVal << 4;
    setVal = setVal & 0x00000FF0;
    tbr = tbr & 0xFFFFF00F;
    tbr = tbr | setVal;
}

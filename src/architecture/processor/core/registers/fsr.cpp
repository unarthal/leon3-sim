#include "architecture/processor/core/registers/fsr.h"

void floating_point_state_register::setFSR(int set_value)
{
    cexc =(unsigned int)(set_value & 0x0000001F);
    aexc = (unsigned int)(set_value & 0x000003E0);
    fcc = (unsigned int)(set_value & 0x00000C00);
    ulow = (unsigned int)(set_value & 0x00001000);
    qne = (unsigned int)(set_value & 0x00002000);
    ftt = (unsigned int)(set_value & 0x0001C000);
    ver = (unsigned int)(set_value & 0x000E0000);
    res = (unsigned int)(set_value & 0x00300000);
    ns = (unsigned int)(set_value & 0x00400000);
    tem = (unsigned int)(set_value & 0x0F800000);
    uhigh = (unsigned int)(set_value & 0x30000000);
    rd = (unsigned int)(set_value & 0xC0000000);
}

unsigned int floating_point_state_register::getFSR(void)
{
    unsigned int ret_val;
    ret_val = (rd<<30)      |
              (uhigh<<28)   |
              (tem<<23)     |
              (ns<<22)      |
              (res<<20)     |
              (ver<<17)     |
              (ftt<<14)     |
              (qne<<13)     |
              (ulow<<12)    |
              (fcc<<10)     |
              (aexc<<5)     |
              (cexc<<0);
    return ret_val;

}

int floating_point_state_register::getCexc()
{
    return this->cexc;
}

void floating_point_state_register::setCexc(int cexc)
{
    this->cexc = cexc;
}

int floating_point_state_register::getAexc()
{
    return this->aexc;
}

void floating_point_state_register::setAexc(int aexc)
{
    this->aexc = aexc;
}

int floating_point_state_register::getFcc()
{
    return this->fcc;
}

void floating_point_state_register::setFcc(int fcc)
{
    this->fcc = fcc;
}

int floating_point_state_register::getUlow()
{
    return this->ulow;
}

void floating_point_state_register::setUlow(int ulow)
{
    this->ulow = ulow;
}

int floating_point_state_register::getQne()
{
    return this->qne;
}

void floating_point_state_register::setQne(int qne)
{
    this->qne = qne;
}

int floating_point_state_register::getFtt()
{
    return this->ftt;
}

void floating_point_state_register::setFtt(int ftt)
{
    this->ftt = ftt;
}

int floating_point_state_register::getVer()
{
    return this->ver;
}

void floating_point_state_register::setVer(int ver)
{
    this->ver = ver;
}

int floating_point_state_register::getRes()
{
    return this->res;
}

void floating_point_state_register::setRes(int res)
{
    this->res = res;
}

int floating_point_state_register::getNs()
{
    return this->ns;
}

void floating_point_state_register::setNs(int ns)
{
    this->ns = ns;
}

int floating_point_state_register::getTem()
{
    return this->tem;
}

void floating_point_state_register::setTem(int tem)
{
    this->tem = tem;
}

int floating_point_state_register::getUhigh()
{
    return this->uhigh;
}

void floating_point_state_register::setUhigh(int uhigh)
{
    this->uhigh = uhigh;
}

int floating_point_state_register::getRd()
{
    return this->rd;
}

void floating_point_state_register::setRd(int rd)
{
    this->rd = rd;
}



#include "psr.h"

        int processor_status_register:: getPSR()
        {
            int psrVal=0;
            int tmp=0;
            tmp=getImpl(); psrVal = (tmp<<28)|psrVal;
            tmp=getVer(); psrVal = (tmp<<24)|psrVal;
            tmp=getN(); psrVal = (tmp<<23)|psrVal;
            tmp=getZ(); psrVal = (tmp<<22)|psrVal;
            tmp=getV(); psrVal = (tmp<<21)|psrVal;
            tmp=getC(); psrVal = (tmp<<20)|psrVal;
            tmp=getReserved(); psrVal = (tmp<<14)|psrVal;
            tmp=getEc(); psrVal = (tmp<<13)|psrVal;
            tmp=getEf(); psrVal = (tmp<<12)|psrVal;
            tmp=getPil(); psrVal = (tmp<<8)|psrVal;
            tmp=getS(); psrVal = (tmp<<7)|psrVal;
            tmp=getPs(); psrVal = (tmp<<6)|psrVal;
            tmp=getEt(); psrVal = (tmp<<5)|psrVal;
            tmp=getCwp(); psrVal = (tmp)|psrVal;
            return psrVal;    
        }

        void processor_status_register:: setPSR(int setVal)
        {
            // setImpl(((unsigned int)setVal>>28)); 
            // setVer(((unsigned int)(setVal<<4)>>28)); 
            // setN(((unsigned int)(setVal<<8)>>30));
            // setZ(((unsigned int)(setVal<<9)>>30));
            // setV(((unsigned int)(setVal<<10)>>30));
            // setC(((unsigned int)(setVal<<11)>>30));
            // setReserved(((unsigned int)(setVal<<12)>>26));
            // setEc(((unsigned int)(setVal<<18)>>31)); 
            // setEf(((unsigned int)(setVal<<19)>>31));
            // setPil(((unsigned int)(setVal<<20)>>28));
            // setS(((unsigned int)(setVal<<24)>>30));
            // setPs(((unsigned int)(setVal<<25)>>30)); 
            // setEt(((unsigned int)(setVal<<26)>>30));
            // setCwp(((unsigned int)(setVal<<27)>>28));
            /// Above was WRONG
            setImpl(((unsigned int)setVal>>28)); 
            setVer(((unsigned int)(setVal<<4)>>28)); 
            setN(((unsigned int)(setVal<<8)>>31));
            setZ(((unsigned int)(setVal<<9)>>31));
            setV(((unsigned int)(setVal<<10)>>31));
            setC(((unsigned int)(setVal<<11)>>31));
            setReserved(((unsigned int)(setVal<<12)>>26));
            setEc(((unsigned int)(setVal<<18)>>31)); 
            setEf(((unsigned int)(setVal<<19)>>31));
            setPil(((unsigned int)(setVal<<20)>>28));
            setS(((unsigned int)(setVal<<24)>>31));
            setPs(((unsigned int)(setVal<<25)>>31)); 
            setEt(((unsigned int)(setVal<<26)>>31));
            setCwp(((unsigned int)(setVal<<27)>>27));
                
        }
        


        int processor_status_register:: getCwp()
        {
            return this->cwp;
        }

        void processor_status_register:: setCwp(int cwp)
        {
            this->cwp = cwp;

        }

        int processor_status_register :: getEt()
        {
            return this->et;
        }

        void processor_status_register:: setEt(int et)
        {
            this->et = et;
        }

        int processor_status_register::getPs()
        {
            return this->ps;
        }

        void processor_status_register::setPs(int ps)
        {
            this->ps = ps;
        }

        int processor_status_register::getS()
        {
            return this->s;
        }

        void processor_status_register::setS(int s)
        {
            this->s = s;
        }

        int processor_status_register::getPil()
        {
            return this->pil;
        }

        void processor_status_register::setPil(int pil)
        {
            this->pil = pil;
        }

        int processor_status_register::getEf()
        {
            return this->ef;
        }

        void processor_status_register::setEf(int ef)
        {
            this->ef = ef;
        }

        int processor_status_register::getEc()
        {
            return this->ec;
        }

        void processor_status_register::setEc(int ec)
        {
            this->ec = ec;
        }

        int processor_status_register::getReserved()
        {
            return this->reserved;
        }

        void processor_status_register::setReserved(int reserved)
        {
            this->reserved = reserved;
        }

        int processor_status_register::getC()
        {
            return this->c;
        }

        void processor_status_register::setC(int c)
        {
            this->c = c;
        }

        int processor_status_register::getV()
        {
            return this->v;
        }

        void processor_status_register::setV(int v)
        {
            this->v = v;
        }

        int processor_status_register::getZ()
        {
            return this->z;
        }

        void processor_status_register::setZ(int z)
        {
            this->z = z;
        }

        int processor_status_register::getN()
        {
            return this->n;
        }

        void processor_status_register::setN(int n)
        {
            this->n = n;
        }

        int processor_status_register::getVer()
        {
            return this->ver;
        }

        void processor_status_register::setVer(int ver)
        {
            this->ver = ver;
        }

        int processor_status_register::getImpl()
        {
            return this->impl;
        }

        void processor_status_register::setImpl(int impl)
        {
            this->impl = impl;
        }

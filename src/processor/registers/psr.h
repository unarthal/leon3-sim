#ifndef PSR_H
#define PSR_H

class processor_status_register {    
    private:
        unsigned int cwp;
        unsigned int et;
        unsigned int ps;
        unsigned int s;
        unsigned int pil;
        unsigned int ef;
        unsigned int ec;
        unsigned int reserved;

        unsigned int c;
        unsigned int v;
        unsigned int z;
        unsigned int n;

        unsigned int ver;
        unsigned int impl;

    public:
        int getCwp();
        
        void setCwp(int cwp);

        int getEt();

        void setEt(int et);

        int getPs();

        void setPs(int ps);

        int getS();

        void setS(int s);

        int getPil();

        void setPil(int pil);

        int getEf();

        void setEf(int ef);

        int getEc();

        void setEc(int ec);

        int getReserved();

        void setReserved(int reserved);

        int getC();

        void setC(int c);

        int getV();

        void setV(int v);

        int getZ();

        void setZ(int z);

        int getN();

        void setN(int n);

        int getVer();

        void setVer(int ver);
        int getImpl();

        void setImpl(int impl);

        int getPSR(void);
        void setPSR(int);

};

#endif

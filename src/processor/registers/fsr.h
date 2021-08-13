#ifndef FSR_H 
#define FSR_H

class floating_point_state_register
{
    private:
        unsigned int cexc;
        unsigned int aexc;
        unsigned int fcc;
        unsigned int ulow;
        unsigned int qne;
        unsigned int ftt;
        unsigned int ver;
        unsigned int res;
        unsigned int ns;
        unsigned int tem;
        unsigned int uhigh;
        unsigned int rd;

    public:
        int setFSR(int set_value);
        unsigned int getFSR(void);
        int getCexc();
        void setCexc(int cexc);
        int getAexc();
        void setAexc(int aexc);
        int getFcc();
        void setFcc(int fcc);
        int getUlow();
        void setUlow(int ulow);
        int getQne();
        void setQne(int qne);
        int getFtt();
        void setFtt(int ftt);
        int getVer();
        void setVer(int ver);
        int getRes();
        void setRes(int res);
        int getNs();
        void setNs(int ns);
        int getTem();
        void setTem(int tem);
        int getUhigh();
        void setUhigh(int uhigh);
        int getRd();
        void setRd(int rd);

}__attribute__ ((__packed__));

#endif

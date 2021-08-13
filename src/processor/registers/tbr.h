#ifndef TBR_H
#define TBR_H

class trap_base_register{
    int tbr;

    public:
        
        int getTbr(void);
        void setTbr(int setVal);

        int getTba(void);
        void setTba(int setVal);
        
        int getTt(void);
        void setTt(int setVal);
};

#endif

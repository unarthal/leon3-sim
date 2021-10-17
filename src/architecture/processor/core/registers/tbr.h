#ifndef SRC_ARCHITECTURE_PROCESSOR_CORE_REGISTERS_TBR_H_
#define SRC_ARCHITECTURE_PROCESSOR_CORE_REGISTERS_TBR_H_

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

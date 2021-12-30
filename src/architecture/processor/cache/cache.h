#ifndef SRC_ARCHITECTURE_PROCESSOR_CACHE_CACHE_H_
#define SRC_ARCHITECTURE_PROCESSOR_CACHE_CACHE_H_

#include "architecture/element.h"
#include "architecture/memory/memory.h"
#include "architecture/interface.h"
#include "architecture/constants_typedefs.h"

#include <queue>
class core;


class simplecacheevent : public event
{
private:
	memorymessage* m_msg;
public:
	simplecacheevent(memorymessage* x_msg, clockType x_eventTime);
	~simplecacheevent();
	memorymessage* getMemoryMessage();
	void setMemoryMessage(memorymessage* x_msg);
};

struct tag{
    unsigned int tag_value;
    bool tag_lrr;
    bool tag_lock;
    bool tag_valid[9];
};

enum cacheType {DIRECTMAPPED, FULLYASSOCIATIVE, SETASSOCIATIVE};
enum replacementPolicy { LRU, LFU, FIFO };

class cache: public element 
{
    private:
        core* m_containingCore;
        interface* m_cache_upperlevel_interface;
        interface* m_upperlevel_cache_interface;
        interface* m_cache_lowerlevel_interface;
        interface* m_lowerlevel_cache_interface;
        element* m_lowerlevel_element;
        element* m_upperlevel_element;
        clockType m_latency;
        int total_ins;
	    std::priority_queue<simplecacheevent*, std::vector<simplecacheevent*>, eventcompare>* m_eventQueue;

        bool m_hasTrapOccurred;//TODO make private

        int m_cacheSize, m_lineSize, m_setAssociativity;
        int m_nlines, m_nsets, m_offset_bits, m_tag_bits, m_index_bits, m_min_index, m_min_value;
        cacheType m_cacheType;
        replacementPolicy m_replacementPolicy;
        unsigned int m_tag, m_index, m_offset;
        struct tag* m_tagArray;
        char **m_dataArray;
        int *m_counter;
        int m_fifoCounter;
        int m_hitCounter, m_readBytes;

    public:
        cache(core* x_containingCore);
        ~cache();
        void simulateOneCycle();
        void setFetchDecodeInterface(interface* x_fetchStage_decodeStage_interface);

        void setHasTrapOccurred();
        std::string* getStatistics();
        core* getContainingCore();

        interface* getCacheUpperlevelInterface();
        void setCacheUpperlevelInterface(interface* x_cache_upperlevel_interface);
        interface* getUpperlevelCacheInterface();
        void setUpperlevelCacheInterface(interface* x_cache_upperlevel_interface);
        interface* getCacheLowerlevelInterface();
        void setCacheLowerlevelInterface(interface* x_cache_upperlevel_interface);
        interface* getLowerlevelCacheInterface();
        void setLowerlevelCacheInterface(interface* x_cache_upperlevel_interface);
        element* getLowelevelElement();
        void setLowerlevelElement(element* x_lowerlevel_element);
        element* getUpperlevelElement();
        void setUpperLevelElement(element* x_upperlevel_element);

        void setCacheSize(int x_size);
        void setLineSize(int x_linesize);
        void setReplacementPolicy(int x_policy);
        void setCacheType(int x_cachetype);
        void setLatency(int x_latency);
        void setSetAssociativity(int x_setAssociativity);

        char* processCacheMessage(memorymessage* x_msg, int read_bit);
        void initialiseCache();
        void initialiseBits();
        int getCacheMin(int x_val);
        void setCacheCounter(int x_index);
        void getCacheMinCounter(int x_start, int x_end);
        int getHits();
        float getHitRatio();
        void flush();
        void printCache();
};

#endif

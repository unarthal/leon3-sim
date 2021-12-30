#include "architecture/processor/cache/cache.h"
#include "architecture/processor/core/core.h"
#include "architecture/processor/processor.h"
#include "generic/utility.h"
#include "architecture/processor/core/fetchStage.h"
#include "architecture/system.h"
#include<math.h>
#include<iostream>
#include<queue>
#include "architecture/processor/core/memoryStage.h"

void cache::initialiseCache()
{
    m_nlines = m_cacheSize/m_lineSize;
    if(m_setAssociativity == 0)
    {
        m_nsets = 1;
        m_cacheType = FULLYASSOCIATIVE;
    }
    else if(m_setAssociativity == 1)
    {
        m_cacheType = DIRECTMAPPED;
        m_nsets = m_nlines/m_setAssociativity;
    }
    else
    {
        m_cacheType = SETASSOCIATIVE;
        m_nsets = m_nlines/m_setAssociativity;
    }
    m_tagArray = (struct tag*)malloc(sizeof(struct tag)*m_nlines);
    m_counter = (int *)malloc(sizeof(int)*m_nlines);
    m_dataArray = (char **)malloc(sizeof(char *)*m_nlines);
    m_hitCounter = 0;
    for(int i=0;i<m_nlines;i++)
    {
        m_tagArray[i].tag_value = -1;
        for(int j=0;j<9;j++)
        {
            m_tagArray[i].tag_valid[j] = false;
        }

        m_counter[i] = 0;
        m_dataArray[i] = (char *)malloc(sizeof(char)*m_lineSize);
        for(int j=0;j<m_lineSize;j++)
            m_dataArray[i][j] = 0;
    }
    m_min_value = 1000000;
    m_min_index =0;
    if(m_replacementPolicy == FIFO)
        m_min_value = 0;
    m_fifoCounter = 1;
    initialiseBits();
}   

void cache::initialiseBits()
{
    m_offset_bits = log(m_lineSize)/log(2);
    m_index_bits =0;
    if(m_cacheType == DIRECTMAPPED)
    {
        m_index_bits = log(m_nlines)/log(2);
    }
    else if(m_cacheType == SETASSOCIATIVE)
    {
        m_index_bits = log(m_nsets)/log(2);
    }
    m_tag_bits = 32 - m_index_bits - m_offset_bits;
}

int cache::getCacheMin(int x_val)
{
    if(x_val<0) return 1;
    else return x_val;
}

void cache::setCacheCounter(int x_index)
{
    if(m_replacementPolicy == FIFO)
    {
        if(m_counter[x_index] == 0) 
        {
            m_counter[x_index] = m_fifoCounter;
            m_fifoCounter++;
        };
    }
    else if(m_replacementPolicy == LFU)
    {
        m_counter[x_index]++;
    }
    else if(m_replacementPolicy == LRU)
    {
        for(int i = 0; i < m_nlines; i++){
            if(i == x_index){
                m_counter[i] = 7;
            }
            else if(m_tagArray[i].tag_valid[0])
            {
                m_counter[i] = getCacheMin(m_counter[i] - 1);
            }
        }
    }
}

void cache::printCache()
{
    std::cout << hex << "[BEGIN] cache dump" << endl;
    if(this==m_containingCore->getDcache())
    std::cout<<"D-Cache" << endl;
    else
    std::cout<<"I-Cache" << endl;
    for(int i=0;i<m_nlines;i++)
    {
        std::cout<<i<<" : ";
        for(int j=0;j<32;j++)
        {
            printByte(m_dataArray[i][j]);
            std::cout << " ";
        }
        std::cout << endl;
    }
    std::cout << "[END] cache dump" << endl << endl << dec;
}

void cache::getCacheMinCounter(int x_start, int x_end)
{
    int min = 1000000, min_index = x_start;
    for(int i = x_start; i < x_end; i++)
    {
        if( min > m_counter[i])
        {
            min = m_counter[i];
            min_index = i ;
        }
    }
    m_min_index = min_index;
}


int cache::getHits(){
	return m_hitCounter;
}

float cache::getHitRatio(){
    return m_hitCounter/(float)total_ins;
}

void cache::flush()
{
    for(int i=0; i< m_nlines; i++)
    {
        m_tagArray[i].tag_value = -1;
        for(int j=0; j<9; j++)
            m_tagArray->tag_valid[j] = false;
        m_counter[i] = 0;
        for(int j = 0; j< m_lineSize; j++)
            m_dataArray[i][j] = 0;
    }
}

char* cache::processCacheMessage(memorymessage* x_msg, int read_bit)
{
    addrType x_addr = x_msg->getAddress();
    int temp_bytes = x_msg->getNoOfBytes();
    char* temp = x_msg->getValue();

    int u = 32 - m_tag_bits - m_offset_bits - m_index_bits;
    m_tag = x_addr>>(m_index_bits + m_offset_bits);
    m_index = 0;

    if(m_cacheType != FULLYASSOCIATIVE)
    m_index = (x_addr<<(m_tag_bits + u))>>(m_tag_bits+m_offset_bits+u);
    m_offset = (x_addr<<(m_tag_bits+m_index_bits+u)) >> (m_tag_bits + m_index_bits+u);
}

cache::cache(core* x_containingCore) : element ()
{
    m_containingCore = x_containingCore;
    m_cache_lowerlevel_interface = 0;
    m_cache_upperlevel_interface = 0;
    m_upperlevel_cache_interface = 0;
    m_lowerlevel_cache_interface = 0;
    m_hasTrapOccurred = false;
	m_eventQueue = new std::priority_queue<simplecacheevent*, std::vector<simplecacheevent*>, eventcompare>();
    total_ins = 0;
}

cache::~cache(){}

void cache::simulateOneCycle(){
    while ((m_eventQueue->empty()==false && m_eventQueue->top()->getEventTime() <= getClock() && (m_eventQueue->top()->getMemoryMessage()->getMemoryMessageType() == ReadResponse || m_eventQueue->top()->getMemoryMessage()->getMemoryMessageType() == WriteResponse)) && getCacheUpperlevelInterface()->getBusy() == false)
    {
        simplecacheevent* evt = m_eventQueue->top();
        addrType addr = evt->getMemoryMessage()->getAddress();
        int noOfBytes = evt->getMemoryMessage()->getNoOfBytes();
        char* value = evt->getMemoryMessage()->getValue();
        element* originalProducer = evt->getMemoryMessage()->getProducer();

        evt->getMemoryMessage()->setProducer(this);
        evt->getMemoryMessage()->setConsumer(getUpperlevelElement());

        int read_bit =0;
        if(evt->getMemoryMessage()->getMemoryMessageType() == ReadResponse)
            read_bit = 1;
        processCacheMessage(evt->getMemoryMessage(),read_bit);
        int start, end;
        if(m_cacheType == DIRECTMAPPED)
        {
            start = m_index;
            end = m_index+1;
        }
        else if(m_cacheType == FULLYASSOCIATIVE)
        {
            start = 0;
            end = m_nlines;
        }
        else
        {
            start = m_index * m_setAssociativity;
            end = (m_index + 1)* m_setAssociativity;
        }
        int hit_flag = 0; 
        int temp_bytes = evt->getMemoryMessage()->getOrgNoOfBytes();
        int originalBytes;
        int org_offset;
        addrType org_address;
        if(temp_bytes==-1)
        {
            originalBytes = noOfBytes;
            org_offset = m_offset;
            org_address = addr;
        }
        else
        {
            originalBytes = temp_bytes;
            org_offset = evt->getMemoryMessage()->getOffset();
            org_address = evt->getMemoryMessage()->getOrgAddress();
        }
        int index;
        for(int i=start;i<end;i++)
        {
            if(m_tagArray[i].tag_valid[0] && m_tagArray[i].tag_value == m_tag)
            {
                if(m_cacheType!=DIRECTMAPPED)
                    setCacheCounter(i);
                for(int j=(int)m_offset;j<(int)m_offset+noOfBytes;j++)
                {
                    m_tagArray[i].tag_valid[j/4 +1] = true;
                    m_dataArray[i][j] = value[j-(int)m_offset];
                }
                hit_flag=1;
                index=i;
                break;
            }
        }
        if(hit_flag==0)
        {
            for(int i=start;i<end;i++)
            {
                if(!m_tagArray[i].tag_valid[0])
                {
                    if(m_cacheType != DIRECTMAPPED)
                        setCacheCounter(i);
                    m_tagArray[i].tag_valid[0] = true;
                    m_tagArray[i].tag_value = m_tag;
                    for(int j=(int)m_offset;j<(int)m_offset+noOfBytes;j++)
                    {
                        m_tagArray[i].tag_valid[j/4 +1] = true;
                        m_dataArray[i][j] = value[j-(int)m_offset];
                    }
                    index=i;
                    hit_flag = 1;
                    break;
                }
            }
        }
        if(hit_flag == 0)
        {
            getCacheMinCounter(start, end);
            if(m_replacementPolicy == FIFO)
            {
                m_counter[m_min_index] = m_fifoCounter;
                m_fifoCounter++;
            }
            if(m_replacementPolicy == LFU)
            {
                m_counter[m_min_index] = 1;
            }
            m_tagArray[m_min_index].tag_value = m_tag;
            for(int i=1;i<9;i++)
                m_tagArray[m_min_index].tag_valid[i] = false;

            m_tagArray[m_min_index].tag_valid[m_offset/4+1] = true;
            for(int i=(int)m_offset;i<(int)m_offset+noOfBytes;i++)
            {
                m_dataArray[m_min_index][i] = value[i-(int)m_offset];
            }
            index=m_min_index;

            if(m_cacheType!=DIRECTMAPPED && m_replacementPolicy==LRU)
            setCacheCounter(m_min_index);
        }

        char *temp = new char[originalBytes];
        for(int j=0;j<originalBytes;j++)
            temp[j] = m_dataArray[index][j+org_offset];
        
        evt->getMemoryMessage()->setValue(temp);
        evt->getMemoryMessage()->setNoOfBytes(originalBytes);
        evt->getMemoryMessage()->setAddress(org_address);
       

        getCacheUpperlevelInterface()->addPendingMessage(evt->getMemoryMessage());
        getLowerlevelCacheInterface()->setBusy(false);
        getUpperlevelCacheInterface()->setBusy(false);
        m_eventQueue->pop();
        delete evt;
    }

    while (m_eventQueue->empty()==false && m_eventQueue->top()->getEventTime() <= getClock() && (m_eventQueue->top()->getMemoryMessage()->getMemoryMessageType() == Read || m_eventQueue->top()->getMemoryMessage()->getMemoryMessageType() == Write) && getCacheLowerlevelInterface()->getBusy() == false)
    {
        simplecacheevent* evt = m_eventQueue->top();
        addrType addr = evt->getMemoryMessage()->getAddress();
        int noOfBytes = evt->getMemoryMessage()->getNoOfBytes();
        char* value = evt->getMemoryMessage()->getValue();

        evt->getMemoryMessage()->setProducer(this);
        evt->getMemoryMessage()->setConsumer(getLowelevelElement());

        int read_bit = 0;
        if(evt->getMemoryMessage()->getMemoryMessageType() == Read)
            read_bit = 1;
        processCacheMessage(evt->getMemoryMessage(),read_bit);

        int start, end;
        char *temp_value;
        temp_value = (char *)malloc(noOfBytes*sizeof(char));
        if(m_cacheType == DIRECTMAPPED)
        {
            start = m_index;
            end = m_index+1;
        }
        else if(m_cacheType == FULLYASSOCIATIVE)
        {
            start = 0;
            end = m_nlines;
        }
        else
        {
            start = m_index * m_setAssociativity;
            end = (m_index + 1)* m_setAssociativity;
        }
        int hit_flag=0;
        if(read_bit==1)
        {
            for(int i=start;i<end;i++)
            {
                if(m_tagArray[i].tag_valid[0] && m_tagArray[i].tag_value == m_tag && m_tagArray[i].tag_valid[(int)m_offset/4 + 1])
                {
                    if(noOfBytes==8 && !m_tagArray[i].tag_valid[(int)m_offset/4 + 2])
                        break;

                    m_hitCounter++;
                    if(m_cacheType!=DIRECTMAPPED)
                        setCacheCounter(i);

                    for(int j=0;j<noOfBytes;j++)
                        temp_value[j] = m_dataArray[i][j+(int)m_offset];
                    evt->getMemoryMessage()->setMemoryMessageType(ReadResponse);
                    evt->getMemoryMessage()->setConsumer(m_upperlevel_element);
                    evt->getMemoryMessage()->setValue(temp_value);
                    getCacheUpperlevelInterface()->addPendingMessage(evt->getMemoryMessage());
                    hit_flag=1;
                    break;
                }
            }
        }
        else
        {
            for(int i=start;i<end;i++)
            {
                if(m_tagArray[i].tag_valid[0] && m_tagArray[i].tag_value == m_tag && m_tagArray[i].tag_valid[(int)m_offset/4 + 1])
                {
                    if(noOfBytes==8 && !m_tagArray[i].tag_valid[(int)m_offset/4 + 2])
                        break;
                    
                    for(int j=0;j<noOfBytes;j++)
                    {
                        m_dataArray[i][j+(int)m_offset] = value[j];
                    }
                    m_hitCounter++;
                    evt->getMemoryMessage()->setMemoryMessageType(WriteResponse);
                    evt->getMemoryMessage()->setConsumer(m_upperlevel_element);
                    getCacheUpperlevelInterface()->addPendingMessage(evt->getMemoryMessage());
                    break;
                }
            }
        }

        if(hit_flag==1)
        {
            getLowerlevelCacheInterface()->setBusy(false);
            getUpperlevelCacheInterface()->setBusy(false);
        }
        else
        {
            if(read_bit==1)
            {
                if(noOfBytes<4)
                {
                    evt->getMemoryMessage()->setOrgNoOfBytes(noOfBytes);
                    evt->getMemoryMessage()->setNoOfBytes(4);
                    evt->getMemoryMessage()->setAddress(((int)addr/4)*4);
                    evt->getMemoryMessage()->setOffset(m_offset);
                    evt->getMemoryMessage()->setOrgAddress(addr);
                }
                evt->getMemoryMessage()->setMemoryMessageType(Read);
                evt->getMemoryMessage()->setConsumer(m_lowerlevel_element);
                getCacheLowerlevelInterface()->addPendingMessage(evt->getMemoryMessage());
            }
            else
            {
                if(noOfBytes<4)
                {
                    evt->getMemoryMessage()->setOffset(m_offset);
                    evt->getMemoryMessage()->setOrgNoOfBytes(noOfBytes);
                    evt->getMemoryMessage()->setOrgAddress(addr);
                }
                evt->getMemoryMessage()->setMemoryMessageType(Write);
                evt->getMemoryMessage()->setConsumer(m_lowerlevel_element);
                getCacheLowerlevelInterface()->addPendingMessage(evt->getMemoryMessage());
            }
        } 
        m_eventQueue->pop();
        delete evt;
    }
    
    while(getLowerlevelCacheInterface()->getBusy() == false && getLowerlevelCacheInterface()->doesElementHaveAnyPendingMessage(this) == true)
    {
        memorymessage* msg = (memorymessage*) (getLowerlevelCacheInterface()->popElementsPendingMessage(this));
        m_eventQueue->push(new simplecacheevent(msg, getClock() + m_latency - 1));
        getLowerlevelCacheInterface()->setBusy(true); 
    }

    while(getUpperlevelCacheInterface()->getBusy() == false && getUpperlevelCacheInterface()->doesElementHaveAnyPendingMessage(this) == true)
    {
        memorymessage* msg = (memorymessage*) (getUpperlevelCacheInterface()->popElementsPendingMessage(this));
        m_eventQueue->push(new simplecacheevent(msg, getClock() + m_latency - 1));
        total_ins++;
        getUpperlevelCacheInterface()->setBusy(true);
    }
}

interface* cache::getCacheLowerlevelInterface(){
    return m_cache_lowerlevel_interface;
}

void cache::setCacheLowerlevelInterface(interface* x_cache_upperlevel_interface){
    m_cache_lowerlevel_interface = x_cache_upperlevel_interface;
}

interface* cache::getCacheUpperlevelInterface(){
    return m_cache_upperlevel_interface;
}

void cache::setCacheUpperlevelInterface(interface* x_cache_upperlevel_interface){
    m_cache_upperlevel_interface = x_cache_upperlevel_interface;
}

interface* cache::getUpperlevelCacheInterface()
{
    return m_upperlevel_cache_interface;
}
void cache::setUpperlevelCacheInterface(interface* x_upperlevel_cache_interface)
{
    m_upperlevel_cache_interface = x_upperlevel_cache_interface;
}

interface* cache::getLowerlevelCacheInterface()
{
    return m_lowerlevel_cache_interface;
}
void cache::setLowerlevelCacheInterface(interface* x_lowerlevel_cache_interface)
{
    m_lowerlevel_cache_interface = x_lowerlevel_cache_interface;
}
        

void cache::setHasTrapOccurred()
{
	m_hasTrapOccurred = true;
}

std::string* cache::getStatistics()
{
	return 0;
}

core* cache::getContainingCore()
{
	return m_containingCore;
}

// int is_mem_address_not_aligned(unsigned int memoryAddress, int alignment)
// {
//     switch(alignment)
//     {
//         case 2: if((((unsigned int)(memoryAddress / 2)) * 2) == memoryAddress) return 0; else return 1;
//         case 4: if((((unsigned int)(memoryAddress / 4)) * 4) == memoryAddress) return 0; else return 1;
//         case 8: if((((unsigned int)(memoryAddress / 8)) * 8) == memoryAddress) return 0; else return 1;
//     }
//     return 0;
// }


simplecacheevent::simplecacheevent(memorymessage* x_msg, clockType x_eventTime)
{

	m_msg = x_msg;
    setEventTime(x_eventTime);
}

simplecacheevent::~simplecacheevent()
{

}

memorymessage* simplecacheevent::getMemoryMessage()
{
	return m_msg;
}

void simplecacheevent::setMemoryMessage(memorymessage* x_msg)
{
	m_msg = x_msg;
}

element* cache::getLowelevelElement()
{
    return m_lowerlevel_element;
}

void cache::setLowerlevelElement(element* x_lowerlevel_element)
{
    m_lowerlevel_element = x_lowerlevel_element;
}

element* cache::getUpperlevelElement()
{
    return m_upperlevel_element;
}

void cache::setUpperLevelElement(element* x_upperlevel_element)
{
    m_upperlevel_element = x_upperlevel_element;
}

void cache::setCacheSize(int x_size){ 
    m_cacheSize = x_size;
}

void cache::setLineSize(int x_linesize){
    m_lineSize = x_linesize;
}

void cache::setReplacementPolicy(int x_policy){
    m_replacementPolicy = (replacementPolicy)x_policy;
}
void cache::setCacheType(int x_cachetype){
}
void cache::setLatency(int x_latency){
    m_latency = (clockType)x_latency;
}
void cache::setSetAssociativity(int x_setAssociativity){
    m_setAssociativity = x_setAssociativity;
}

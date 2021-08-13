#ifndef MEMORYINTERFACE_H
#define MEMORYINTERFACE_H


#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include "Element.h"
#include "Interface.h"
#include "interface/Event.h"

using namespace std;

extern long long unsigned int clock_cycles;


class memoryMessage : public Message {
    public:

    EventType eventType;
    unsigned int addr=0;
    
    char write_byte;
    int write_halfword;
    unsigned long write_word;
    
    
    long long unsigned int deliveryTime=0;
    int size=0;

    memoryMessage (Element* con, EventType evt, 
                    unsigned int ad, char wb, 
                    int whw, unsigned long ww, 
                    int sz, long long unsigned int dT, 
                    int reqid )
    {    
        consumer = con;       
        eventType = evt;
        size=sz;
        addr = ad;
        write_byte = wb;
        write_halfword = whw;
        write_word = ww;
        deliveryTime = dT;
        reqID = reqid;
    }
};

class memoryInterface : public Interface
{
    public:

    priority_queue <memoryMessage> pendingMemoryAccessRequests;
    char read_byte; //1 byte
    char* read_halfword; // 4 bytes
    unsigned long read_word; //8 bytes 

    int write_status;
    
    memoryInterface(Element* pro, Element* con)
    {
        producer = pro;
        consumer = con;
    }

    void processRequests()
    { 
        /// For 1:many Interface
        while ( !pendingMemoryAccessRequests.empty() && 
                pendingMemoryAccessRequests.top().deliveryTime <= clock_cycles)
        {
            producer->handleEvent();
            pendingMemoryAccessRequests.top().consumer->handleEvent();
            pendingMemoryAccessRequests.pop();
        }
    }

    void processDirectRequest()
    { 
        /// For 1:1 interface
    
    }

};


inline bool operator<(const memoryMessage &lhs, const memoryMessage &rhs){
    return lhs.deliveryTime > rhs.deliveryTime;
}

#endif

///// Not using now instead, Now using Interfaces 

#ifndef Event_H
#define Event_H
#include<iostream>

using namespace std;




enum EventType {   Multiply, CacheRead, CacheResponse, MemoryRead, MemoryResponse, Exception, CacheWrite, MemoryWrite};


class Event {
    public:
    long long unsigned int eventTime;
    Element* requestingElement;
    Element* processingElement;
    EventType eventType;

    Event (long long unsigned int evt, Element* reqE, Element* proE, EventType evtyp ) {
        eventTime = evt;
        requestingElement = reqE;
        processingElement = proE;
        eventType = evtyp;
    }


};


#endif
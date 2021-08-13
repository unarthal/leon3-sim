///// Not using now instead, Now using Interfaces 

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H


#include <queue>
#include<iostream>
#include"Event.h"
#include "sim_globals.h"
using namespace std;


class eventQueue{
    public:
    priority_queue<Event> eventQ;

    void processEvents(){
        while(!eventQ.empty() && eventQ.top().eventTime >= clock_cycles ){
            Event tele = eventQ.top();
            // if(tele.eventType == FE_event){
            //     // cout<<"FE_event"<<clock_cycles<<" "<<eventQ.top().eventTime<<"\n";
            //     // tele.processingElement->handleEvent();
            // }
            eventQ.pop();
        }
    }


};

inline bool operator<(const Event &lhs, const Event &rhs){
    return lhs.eventTime > rhs.eventTime;
}



#endif

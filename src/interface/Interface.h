#ifndef INTERFACE_H
#define INTERFACE_H


#include<iostream>
#include<vector>
#include"Element.h"
#include <queue>
using namespace std;


class Message{
    public:
    Element* consumer; /// For (1:many) interface
    char* msg;
    int reqID=-1;
    
};

class Interface{
    public:
    Element* producer;
    Element* consumer; /// For (1:1) Binary Inteface instead for (1:many) interfaces consumer ponter is in the message itself
    queue<Message> pending_messsages;

    int reqID=-1;
    bool busy;

};

#endif

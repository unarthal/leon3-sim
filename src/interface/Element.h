#ifndef ELEMENT_H
#define ELEMENT_H

#include<iostream>

using namespace std;

class Element
{
    public:
        virtual void perform() = 0;
        virtual void handleEvent() = 0;
};

#endif

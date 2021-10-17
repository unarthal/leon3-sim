#ifndef SRC_ARCHITECTURE_ELEMENT_H_
#define SRC_ARCHITECTURE_ELEMENT_H_

#include <vector>
#include <queue>
#include "architecture/constants_typedefs.h"
#include <string>

using namespace std;

class interface;

class event
{
private:
	clockType m_eventTime;
public:
	clockType getEventTime();
	void setEventTime(clockType x_eventTime);
};

class eventcompare /* elements which require a more complex ordering of events may implement their own compare functions */
{
public:
	bool operator()(event* e1, event* e2);
};

class element
{
private:
	vector<interface*>* m_inputInterfaces;
	vector<interface*>* m_outputInterfaces;

public:
	element();
	~element();
	virtual void simulateOneCycle() = 0;
	virtual std::string* getStatistics() = 0;
	vector<interface*>* getInputInterfaces();
	vector<interface*>* getOutputInterfaces();
};

#endif

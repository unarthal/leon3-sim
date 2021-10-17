#include "architecture/element.h"

element::element()
{
	m_inputInterfaces = new vector<interface*>();
	m_outputInterfaces = new vector<interface*>();
}

element::~element()
{
	delete m_inputInterfaces;
	delete m_outputInterfaces;
}

vector<interface*>* element::getInputInterfaces()
{
	return m_inputInterfaces;
}

vector<interface*>* element::getOutputInterfaces()
{
	return m_outputInterfaces;
}

clockType event::getEventTime()
{
	return m_eventTime;
}

void event::setEventTime(clockType x_eventTime)
{
	m_eventTime = x_eventTime;
}

bool eventcompare::operator()(event* e1, event* e2)
{
	return e1->getEventTime() < e2->getEventTime();
}

#include "architecture/interface.h"
#include "architecture/element.h"
#include <iostream>

message::message()
{
	m_producer = 0;
	m_consumer = 0;
	m_reqID = -1;
}

message::message(element* x_producer, element* x_consumer)
{
	m_producer = x_producer;
	m_consumer = x_consumer;
	m_reqID = -1;
}

message::~message()
{

}

element* message::getProducer()
{
	return m_producer;
}

void message::setProducer(element* x_producer)
{
	m_producer = x_producer;
}

element* message::getConsumer()
{
	return m_consumer;
}

void message::setConsumer(element* x_consumer)
{
	m_consumer = x_consumer;
}

int message::getReqID()
{
	return m_reqID;
}

void message::setReqID(int x_reqID)
{
	m_reqID = x_reqID;
}

interface::interface()
{
	m_pendingMessages = new std::vector<message*>();
	m_busy = false;
}

interface::~interface()
{
	delete m_pendingMessages;
}

void interface::addPendingMessage(message* x_msg)
{
	m_pendingMessages->push_back(x_msg);
	if(m_pendingMessages->size() > 10000)
	{
		cout << "\npending message queue growing too large. possible issue.\n\n";
	}
}

bool interface::doesElementHaveAnyPendingMessage(element* x_consumer)
{
	for(unsigned int i = 0; i < m_pendingMessages->size(); i++)
	{
		if(m_pendingMessages->at(i)->getConsumer() == x_consumer)
			return true;
	}
	return false;
}

message* interface::peekElementsPendingMessage(element* x_consumer)
{
	for(unsigned int i = 0; i < m_pendingMessages->size(); i++)
	{
		if(m_pendingMessages->at(i)->getConsumer() == x_consumer)
			return m_pendingMessages->at(i);
	}
	return 0;
}

message* interface::popElementsPendingMessage(element* x_consumer)
{
	for(unsigned int i = 0; m_pendingMessages->size(); i++)
	{
		if(m_pendingMessages->at(i)->getConsumer() == x_consumer)
		{
			message* msg = m_pendingMessages->at(i);
			m_pendingMessages->erase(m_pendingMessages->begin()+i);
			return msg;
		}
	}
	return 0;
}

void interface::flushAllPendingMessages(element* x_consumer)
{
	for(unsigned int i = 0; m_pendingMessages->size(); i++)
	{
		if(m_pendingMessages->at(i)->getConsumer() == x_consumer)
		{
			m_pendingMessages->erase(m_pendingMessages->begin()+i);
		}
	}
}

bool interface::getBusy()
{
	return m_busy;
}

void interface::setBusy(bool x_busy)
{
	m_busy = x_busy;
}

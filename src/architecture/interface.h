#ifndef SRC_ARCHITECTURE_INTERFACE_H_
#define SRC_ARCHITECTURE_INTERFACE_H_

#include <vector>

class element;

class message
{
private:
	element* m_producer;
	element* m_consumer;
	int m_reqID=-1; // to help differentiate between multiple outstanding requests

public:
	message();
	message(element* x_producer, element* x_consumer);
	~message();
	element* getProducer();
	void setProducer(element* x_producer);
	element* getConsumer();
	void setConsumer(element* x_consumer);
	int getReqID();
	void setReqID(int x_reqID);
};

class interface
{
private:
    std::vector<message*>* m_pendingMessages;
    bool m_busy;
public:
    interface();
    ~interface();
    void addPendingMessage(message* x_msg);
    bool doesElementHaveAnyPendingMessage(element* x_consumer);
    message* peekElementsPendingMessage(element* x_consumer);
    message* popElementsPendingMessage(element* x_consumer);
    void flushAllPendingMessages(element* x_consumer);
    bool getBusy();
    void setBusy(bool x_busy);
};

#endif

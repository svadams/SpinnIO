//####################################################################//
//SpinnIO - An Open-Source library for spike io to/from a spinnaker board																													
//
//																																																				//
//Copyright (c) <2015>, BABEL project, Plymouth University in collaboration with Manchester University																																								


#include "Threadable.h"
#include "DatabaseIF.h"
#include "SocketIF.h"
#include "EIEIOMessage.h"
#include <deque>
#include <list>
#include <map>
#include <pthread.h>

#ifndef SPINNIO_EIEIO_SENDER_H_
#define SPINNIO_EIEIO_SENDER_H_

using namespace std;

namespace spinnio
{

class EIEIOSender : public Threadable {
public:
	EIEIOSender(int, char*, bool, char*);
	EIEIOSender(int, char*, std::map<int,int>*);
	virtual ~EIEIOSender();
        int getSendQueueSize();
        SocketIF* getSocketPtr();
        std::map<int,int>* getKeyIDMap();
        std::map<int,int>* getIDKeyMap();
        void enableSendQueue();
        void addSpikeToSendQueue(int);
        void closeSendSocket();
protected:
	void InternalThreadEntry();
private:
	pthread_mutex_t send_mutex;
	pthread_cond_t cond;
        DatabaseIF *dbConnection;
        std::map<int, int> *keyIDMap;
        std::map<int, int> *idKeyMap;
        SocketIF *sendSocket;
        std::deque<int > sendQueue;
        bool sendQueueEnabled;
        int sockfd_input;
        struct addrinfo hints;
        struct addrinfo* addr_info;
        void sendMessage(uint8_t*, int);
        
};
}
#endif /* SPINNIO_EIEIO_SENDER_H_ */

//####################################################################//
//SpinnIO - An Open-Source library for spike io to/from a spinnaker board																													
//
//																																																				//
//Copyright (c) <2015>, BABEL project, Plymouth University in collaboration with Manchester University

#include "EIEIOSender.h"
#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>

using namespace std;

namespace spinnio
{
// Standard constructor where socket is created for send
// and toolchain handshake is done to get back key-id mappings
EIEIOSender::EIEIOSender(int sendPort, char* ip, bool dbConn, char* dbPath) {


	printf("Creating EIEIO sender to spiNNaker with port %d and IP %s\n", sendPort, ip);

	if (pthread_mutex_init(&this->send_mutex, NULL) == -1) {
	        fprintf(stderr, "Error initializing send mutex!\n");
	        exit(-1);
	}
	pthread_cond_init(&this->cond, 0);

        // Setup database connection and read if needed


        if (dbConn == true){
            int dbListenPort = 19999;
            // For now the port for connection to spinnaker is hardcoded
            // but this will change in next release
            dbConnection = new DatabaseIF(dbListenPort, (char*)dbPath);
            if (dbConnection->finishedRead()){
               // get key-id and id-key mappings
               this->keyIDMap = dbConnection->getKeyIDMap();
               this->idKeyMap = dbConnection->getIDKeyMap();
               printf("OK to send...Cleaning up dbConnection\n");
               dbConnection = NULL;
            }

        }	

       // set up socket to send packets
       sendSocket = new SocketIF(sendPort);
       this->sockfd_input = sendSocket->getSocket();
       // setup socket for send
       memset(&this->hints,0,sizeof(this->hints));
       this->hints.ai_family = AF_UNSPEC;
       this->hints.ai_socktype = SOCK_DGRAM;
       this->hints.ai_protocol = 0;
       this->hints.ai_flags = 0;
       this->addr_info = 0;

       if (getaddrinfo(ip, NULL, &(this->hints), &(this->addr_info)) != 0) {
           fprintf(stderr, "Could not resolve hostname, exiting\n");
           exit(-1);
       }
       ((struct sockaddr_in *) this->addr_info->ai_addr)->sin_port = htons(sendPort);

       this->sendQueueEnabled = false;
}


// Alternate constructor without DB handshake for when this has already been done
// by a Receiver. Accepts the ID-Key map as an argument

EIEIOSender::EIEIOSender(int sendPort,char* ip, std::map<int, int>* keymap) {


	printf("Creating EIEIO sender to spiNNaker with port %d and IP %s\n", sendPort, ip);

	if (pthread_mutex_init(&this->send_mutex, NULL) == -1) {
	        fprintf(stderr, "Error initializing send mutex!\n");
	        exit(-1);
	}
	pthread_cond_init(&this->cond, 0);


       // set up socket to send packets
       sendSocket = new SocketIF(sendPort);
       this->sockfd_input = sendSocket->getSocket();
       // setup socket for send
       memset(&this->hints,0,sizeof(this->hints));
       this->hints.ai_family = AF_UNSPEC;
       this->hints.ai_socktype = SOCK_DGRAM;
       this->hints.ai_protocol = 0;
       this->hints.ai_flags = 0;
       this->addr_info = 0;

       if (getaddrinfo(ip, NULL, &(this->hints), &(this->addr_info)) != 0) {
           fprintf(stderr, "Could not resolve hostname, exiting\n");
           exit(-1);
       }
       ((struct sockaddr_in *) this->addr_info->ai_addr)->sin_port = htons(sendPort);

       // As this is a sender we only use the neuron_id - key map

       this->idKeyMap = keymap;

       this->sendQueueEnabled = false;
	
}


void EIEIOSender::InternalThreadEntry(){

        spike outputSpikes[MAX_SPIKE_COUNT];
        int n = 0;
        EIEIOMessage* tmpMsg = 0;

            

	while (1) {                             // for ever ever, ever ever.

            // check if the queue is in use and has something in it
            // before attempting to take spikes from it
            if ((this->sendQueueEnabled) && (this->getSendQueueSize() > 0)){   
               // keep adding spikes to the list up to max event count or until
               // send queue becomes empty 
               while ((n < MAX_SPIKE_COUNT) && (this->getSendQueueSize() > 0)){   
                  //printf("n is %d\n",n);
                  // get the next spike from the queue and convert to a key
                  pthread_mutex_lock(&this->send_mutex); 
                  int id = this->sendQueue.front();
                  outputSpikes[n].key = (*(this->idKeyMap))[id] & OUT_MASK;
                  //printf("key is %x = %i\n",outputSpikes[n].key,id);
                  this->sendQueue.pop_front();
                  pthread_cond_signal(&this->cond);
                  pthread_mutex_unlock(&this->send_mutex);
                  n++;
               }
               // Generate and send a packet
               EIEIOMessage* tmpMsg = new EIEIOMessage(uint8_t(n), BASIC,
                                          TYPE_32_K, 0, false, false, false);
               int byteCount = tmpMsg->writeMessage(outputSpikes, n);
               //printf("Bytes %d\n",byteCount);
               this->sendMessage(tmpMsg->getMessageBuffer(),byteCount);
               n=0;
            }// check queue is ready


        }

}


int EIEIOSender::getSendQueueSize(){
   int size = 0;
   pthread_mutex_lock(&this->send_mutex);
   size = this->sendQueue.size();
   pthread_cond_signal(&this->cond);
   pthread_mutex_unlock(&this->send_mutex);

   return size;

}

SocketIF* EIEIOSender::getSocketPtr(){

    return this->sendSocket;


}

std::map<int, int> *EIEIOSender::getKeyIDMap() {

	return this->keyIDMap;

}

std::map<int, int> *EIEIOSender::getIDKeyMap() {

	return this->idKeyMap;

}


void EIEIOSender::closeSendSocket(){

   this->sendSocket->closeSocket();
}

void EIEIOSender::addSpikeToSendQueue(int neuronID){

    pthread_mutex_lock(&this->send_mutex);
    this->sendQueue.push_back(neuronID);
    pthread_cond_signal(&this->cond);
    pthread_mutex_unlock(&this->send_mutex);


}

void EIEIOSender::enableSendQueue(){

   this->sendQueueEnabled = true;


}

void EIEIOSender::sendMessage(uint8_t* data, int dataSize) {

    if (sendto(this->sockfd_input, data, dataSize, 0,
        this->addr_info->ai_addr, this->addr_info->ai_addrlen) == -1) {
        fprintf(stderr, "Could not send packet \n");
        exit(-1);
    }//else{
      //printf("Packet send OK\n");
    //}
}



EIEIOSender::~EIEIOSender() {
}
}

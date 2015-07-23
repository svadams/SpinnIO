//####################################################################//
//SpinnIO - An Open-Source library for spike io to/from a spinnaker board																													
//
//																																																				//
//Copyright (c) <2015>, BABEL project, Plymouth University in collaboration with Manchester University

#include "EIEIOReceiver.h"
#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>

using namespace std;

namespace spinnio
{
// Standard constructor where socket is created for receive
// and toolchain handshake is done to get back key-id mappings
EIEIOReceiver::EIEIOReceiver(int spinnPort, char* ip, bool dbConn, char* dbPath) {


	printf("Creating EIEIO receiver from spiNNaker with port %d and IP %s\n", spinnPort, ip);

	if (pthread_mutex_init(&this->recvr_mutex, NULL) == -1) {
	        fprintf(stderr, "Error initializing recvr mutex!\n");
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
               printf("Cleaning up dbConnection\n");
               dbConnection = NULL;
            }

        }	

       // set up socket to receive packets
       recvSocket = new SocketIF(spinnPort, (char*)ip);

       this->sockfd_input = recvSocket->getSocket();
       this->si_other = recvSocket->getSiOther();
       this->addr_len_input = recvSocket->getAddrLenInput();
}

// Alternate constructor without DB handshake for when this has already been done
// by a sender. Accepts the Key-ID map as an argument

EIEIOReceiver::EIEIOReceiver(int spinnPort, char* ip, std::map<int, int>* keymap) {

	printf("Creating EIEIO receiver from spiNNaker with port %d and IP %s\n", spinnPort, ip);

	if (pthread_mutex_init(&this->recvr_mutex, NULL) == -1) {
	        fprintf(stderr, "Error initializing recvr mutex!\n");
	        exit(-1);
	}
	pthread_cond_init(&this->cond, 0);


       // set up socket to receive packets
       recvSocket = new SocketIF(spinnPort, (char*)ip);

       this->sockfd_input = recvSocket->getSocket();
       this->si_other = recvSocket->getSiOther();
       this->addr_len_input = recvSocket->getAddrLenInput();

       // As this is a Receiver we only use the Key-ID map
       this->keyIDMap = keymap;
	
}


void EIEIOReceiver::InternalThreadEntry(){

       unsigned char buffer_input[3000];
       //int numPckts = 0;

       // DEBUG check the key map :)
            
       //for (std::map<int, int>:: iterator i = this->keyIDMap->begin(); i != this->keyIDMap->end(); ++i){
       //   printf("key %d id %d\n", i->first, i->second);
       //}
            
            
	while (1) {                             // for ever ever, ever ever.


            
            // read from socket and push onto recv queue

            int numbytes_input = recvfrom(this->sockfd_input, (char *) buffer_input,
                                 sizeof(buffer_input), 0, (sockaddr*) &this->si_other,
                                 (socklen_t*) &this->addr_len_input);
            if (numbytes_input == -1) {
               fprintf(stderr, "Packet not received, exiting\n");
               exit(-1);
            } else if (numbytes_input < 9) {
               fprintf(stderr, "Error - packet too short\n");
               continue;
            } else {
                // packet is OK
                //numPckts++;
                //printf("got a packet, total so far is %d\n", numPckts);
		//list<pair<int, int> > data;
                // Decode eieio message
		struct eieio_message* new_message = new eieio_message();
		new_message->header.count = buffer_input[0];
		new_message->header.p = ((buffer_input[1] >> 7) & 1);
		new_message->header.f = (messageFormat)((buffer_input[1] >> 6) & 1);
		new_message->header.d = ((buffer_input[1] >> 5) & 1);
		new_message->header.t = ((buffer_input[1] >> 4) & 1);
		new_message->header.type = (messageType)((buffer_input[1] >> 2) & 3);
		new_message->header.tag = (buffer_input[1] & 3);
		new_message->data = (unsigned char *) malloc(numbytes_input - 2);
		memcpy(new_message->data, &buffer_input[2], numbytes_input - 2);

               // convert message to a list of (time, nrn id) pairs
               // and put onto receive List
               this->convertEIEIOMessage(new_message);
               // clean up message data
	       free(new_message->data);

            }


        }

}


void EIEIOReceiver::convertEIEIOMessage(eieio_message* message){


	//check that its a data message
	if (message->header.f != 0 or message->header.p != 0 or message->header.d != 1 
                                   or message->header.t != 1 or message->header.type != 2){
           printf("this packet was determined to be a "
                  "command packet. therefore not processing it.");
           printf("eieio message: p=%i f=%i d=%i t=%i type=%i tag=%i count=%i\n",
                  message->header.p, message->header.f, message->header.d,
                  message->header.t, message->header.type, message->header.tag,
                  message->header.count);
        } else {
           uint time = (message->data[3] << 24) |
                       (message->data[2] << 16) |
                       (message->data[1] << 8) |
                       (message->data[0]);

           for (int position = 0; position < message->header.count; position++){
              int data_position = (position * SIZE_OF_KEY) + 4;
              int key = (message->data[data_position + 3] << 24) |
                        (message->data[data_position + 2] << 16) |
                        (message->data[data_position + 1] << 8) |
                        (message->data[data_position]);
              int neuron_id = (*(this->keyIDMap))[key];
              if (this->keyIDMap->find(key) == this->keyIDMap->end()) {
                 fprintf(stderr, "Missing neuron id for key %d\n", key);
                 continue;
              }
              //fprintf(stderr, "time = %i, key = %i, neuron_id = %i\n", time, key, neuron_id);
              pair<int, int> point(time, neuron_id);
              pthread_mutex_lock(&this->recvr_mutex);
              // Push spike event onto master queue
              this->recvQueue.push_back(point);
              pthread_cond_signal(&this->cond);
              pthread_mutex_unlock(&this->recvr_mutex);
           } // for
        } // if data message
}

int EIEIOReceiver::getRecvQueueSize(){
   int size = 0;
   pthread_mutex_lock(&this->recvr_mutex);
   size = this->recvQueue.size();
   pthread_cond_signal(&this->cond);
   pthread_mutex_unlock(&this->recvr_mutex);

   return size;

}

list<pair<int,int> >* EIEIOReceiver::getRecvQueue(){

   return &(this->recvQueue);

}


SocketIF* EIEIOReceiver::getSocketPtr(){

    return this->recvSocket;


}

std::map<int, int> *EIEIOReceiver::getKeyIDMap() {

	return this->keyIDMap;

}
std::map<int, int> *EIEIOReceiver::getIDKeyMap() {

	return this->idKeyMap;

}

void EIEIOReceiver::closeRecvSocket(){

   this->recvSocket->closeSocket();
}

EIEIOReceiver::~EIEIOReceiver() {
}
}

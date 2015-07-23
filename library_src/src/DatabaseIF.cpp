//####################################################################//
//SpinnIO - An Open-Source library for spike io to/from a spinnaker board																													
//
//																																																				//
//Copyright (c) <2015>, BABEL project, Plymouth University in collaboration with Manchester University																																								


#include <stdio.h>
#include <iostream>
#include <string.h>
#include <map>
#include <sqlite3.h>
#include <algorithm>
#include <deque>
#include <cstdlib>
#include <unistd.h>
#include "DatabaseIF.h"

using namespace std;

struct database_data{
	int no_columns;
	char ** fields;
	char ** attributes;
};

namespace spinnio
{

DatabaseIF::~DatabaseIF() {
}

DatabaseIF::DatabaseIF(int listenPort, char* dbPath) {
        finished = false;
        cout << "Creating database handshaker with port " << listenPort << endl;
	this->sock = new SocketIF(listenPort);
        this->sockfd_input = sock->getSocket();
        cout << "Attempting to handshake with toolchain" << endl;
        receiveNotification();
        // Open database and read Key-ID map
        cout << "Attempting to open database " << dbPath << endl;
        openDatabase(dbPath);
        cout << "Reading key-id maps" << endl;
        // Here we make an assumption that there is only one
        // receving and one sending population and that they are
        // tagged 'spikes_in' and 'spikes_out' respectively
        string sender_pop = "spikes_out";
        string receiver_pop = "spikes_in";
        this->keyIDMap = new std::map<int, int>();
        this->idKeyMap = new std::map<int, int>();
        readDatabase((char*) sender_pop.c_str(), true, (std::map<int, int>*)this->keyIDMap);
        readDatabase((char*) receiver_pop.c_str(), false, (std::map<int, int>*)this->idKeyMap);
        cout << "Attempting to close database" << endl;
        closeDbConnection();
        // send message to toolchain to confirm read
        cout << "Sending acknowledgement to toolchain" << endl;
        sendReadyNotification();
        // close the connection as we've finished with the database
        cout << "Closing database connection" << endl;
        sock->closeSocket();
        finished = true;
}

void DatabaseIF::openDatabase(char* dbPath){
    
    int rc;
    rc = sqlite3_open(dbPath, &this->db);
    if(rc){
       //fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(this->db));
       cerr << "Can't open database: " << sqlite3_errmsg(this->db) << endl;
    }
    else{
      cerr << "Opened database successfully" << endl;
    }
    
}



void DatabaseIF::readDatabase(char* pop, bool keyToID, std::map<int, int>* keyMap) {
        char *sql = sqlite3_mprintf(
        "SELECT n.neuron_id as n_id, n.key as key"
        " FROM key_to_neuron_mapping as n"
        " JOIN Partitionable_vertices as p ON n.vertex_id = p.vertex_id"
        " WHERE p.vertex_label=\"%q\"", pop);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            int neuron_id = sqlite3_column_int(compiled_statment, 0);
            int key = sqlite3_column_int(compiled_statment, 1);
            if (keyToID == true){
               (*keyMap)[key] = neuron_id;
            } else {
               (*keyMap)[neuron_id] = key;
            }
        }
        sqlite3_free(sql);
    } else {
        cerr << "Error reading database: " << sqlite3_errcode(this->db) << " " << sqlite3_errmsg(this->db) << endl;
        exit(-1);
    }
}

std::map<int, int> *DatabaseIF::getKeyIDMap() {

	return this->keyIDMap;

}

std::map<int, int> *DatabaseIF::getIDKeyMap() {

	return this->idKeyMap;

}

void DatabaseIF::closeDbConnection(){
	sqlite3_close(this->db);
}



void DatabaseIF::receiveNotification(){
	socklen_t addr_len_input = sizeof(struct sockaddr_in);
	char sdp_header_len = 26;
	unsigned char buffer_input[1515];
	bool received = false;


	while (!received){
		int numbytes_input = recvfrom(
		    this->sockfd_input, (char *) buffer_input, sizeof(buffer_input), 0,
		    (sockaddr*) &this->response_address, (socklen_t*) &addr_len_input);
		if (numbytes_input == -1) {
                        cerr << "Packet not received, exiting" << endl;

			// will only get here if there's an error getting the input frame
			// off the Ethernet
			exit(-1);
		}
		if (numbytes_input < 2) {
                        cerr << "Error - packet too short" << endl;
			continue;
		}
		received = true;
	}
}

void DatabaseIF::sendReadyNotification(){

	char message[2];
	message[1] = (1 << 6);
	message[0] = 1;
	int length = 2;

	int response = sendto(this->sockfd_input, message, length, 0,
			          (struct sockaddr *) &this->response_address,
			          sizeof(this->response_address));
}

bool DatabaseIF::finishedRead(){
	return this->finished;
}
}// namespace spinnio

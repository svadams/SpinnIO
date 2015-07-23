//####################################################################//
//SpinnIO - An Open-Source library for spike io to/from a spinnaker board																													
//
//																																																				//
//Copyright (c) <2015>, BABEL project, Plymouth University in collaboration with Manchester University																																								


#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>
#include "SocketIF.h"

using namespace std;


namespace spinnio
{

SocketIF::~SocketIF() {
}

// Constructor accepting just port for general 
// connection

SocketIF::SocketIF(int socketPort) {
	this->addr_len_input = sizeof(this->si_other);
        printf("Creating socket with port %d\n", socketPort);
	openSocket(socketPort);
}

// Constructor with port and IP is for the
// board connection to receive from SpiNNaker.
// It does a sendVoidMessage on port 17893
// to ensure SpiNNaker can send


SocketIF::SocketIF(int socketPort, char *ip) {
	this->addr_len_input = sizeof(this->si_other);
        printf("Creating socket with port %d\n", socketPort);
	openSocket(socketPort);
        printf("Checking hostname %s\n", ip);
	if (ip != NULL) {
	    sendVoidMessage(ip, 17893);
	}

}

int SocketIF::getSocket(){

   return this->sockfd_input;

}

socklen_t SocketIF::getAddrLenInput(){

   return this->addr_len_input;
}

sockaddr_in SocketIF::getSiOther(){

   return this->si_other;

}

void SocketIF::openSocket(int port) {
    char portno_input[6];
    printf("Creating socket\n");
    snprintf(portno_input, 6, "%d", port);
    struct addrinfo hints_input;
    bzero(&hints_input, sizeof(hints_input));
    hints_input.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints_input.ai_socktype = SOCK_DGRAM; // type UDP (socket datagram)
    hints_input.ai_flags = AI_PASSIVE; // use my IP

    int rv_input;
    struct addrinfo *servinfo_input;
    if ((rv_input = getaddrinfo(NULL, portno_input, &hints_input,
            &servinfo_input)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_input));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    struct addrinfo *p_input;
    for (p_input = servinfo_input; p_input != NULL; p_input =
            p_input->ai_next) {
        if ((this->sockfd_input = socket(p_input->ai_family,
                                         p_input->ai_socktype,
                                         p_input->ai_protocol))
                == -1) {
            printf("SocketIF: socket");
            perror("SocketIF: socket");
            continue;
        }

        if (bind(this->sockfd_input, p_input->ai_addr,
                 p_input->ai_addrlen) == -1) {
            close(this->sockfd_input);
            printf("SocketIF: bind");
            perror("SocketIF: bind");
            continue;
        }

        break;
    }

    if (p_input == NULL) {
        fprintf(stderr, "SocketIF: failed to bind socket\n");
        printf("SocketIF: failed to bind socket\n");
        exit(-1);
    }

    freeaddrinfo(servinfo_input);
}

void SocketIF::sendVoidMessage(char *hostname, int port) {
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
    struct addrinfo* addr_info = 0;
    if (getaddrinfo(hostname, NULL, &hints, &addr_info) != 0) {
        fprintf(stderr, "Could not resolve hostname, exiting\n");
        exit(-1);
    }
    ((struct sockaddr_in *) addr_info->ai_addr)->sin_port = htons(port);

    char data [1];
    if (sendto(this->sockfd_input, data, 1, 0,
        addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
        fprintf(stderr, "Could not send packet \n");
        exit(-1);
    }
    printf("IP check OK\n");
}



void SocketIF::closeSocket(){
	close(this->sockfd_input);
}


}// namespace spinnio

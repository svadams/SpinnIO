//####################################################################//
//SpinnIO - An Open-Source library for spike io to/from a spinnaker board																													
//
//																																																				//
//Copyright (c) <2015>, BABEL project, Plymouth University in collaboration with Manchester University																																								

#include "EIEIOMessage.h"
#include "SocketIF.h"
#include <sqlite3.h>
#include <map>
#include <string>
#ifndef WIN32
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <ws2tcpip.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)
#define close(sock)
typedef unsigned int uint;
typedef unsigned short ushort;
#endif

#ifndef SPINNIO_DATABASEIF_H_
#define SPINNIO_DATABASEIF_H_

namespace spinnio
{

class DatabaseIF {
        SocketIF *sock;
        int sockfd_input;
	struct sockaddr_in response_address;
	sqlite3 *db;
        std::map<int, int> *keyIDMap;
        std::map<int, int> *idKeyMap;
public:
	virtual ~DatabaseIF();
	DatabaseIF(int, char*);
	void receiveNotification();
	void sendReadyNotification();
	void closeConnection();
        bool finishedRead();
        void openDatabase(char*);
	void readDatabase(char*, bool, std::map<int, int>*);
        std::map<int, int> *getKeyIDMap();
        std::map<int, int> *getIDKeyMap();
	void closeDbConnection();

private:
        bool finished;
};
}
#endif /* SPINNIO_DATABASEIF_H_ */

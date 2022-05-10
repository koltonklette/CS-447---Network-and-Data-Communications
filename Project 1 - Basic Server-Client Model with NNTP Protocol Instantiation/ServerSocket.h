//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//NOTE: Resource from Linux Gazette is cited in supplementary report.

#ifndef ServerSocket_class
#define ServerSocket_class

#include "Socket.h"
using namespace std;


class ServerSocket : private Socket
{
public:

	ServerSocket(int port);
	ServerSocket() {};
	virtual ~ServerSocket();

	const ServerSocket& operator << (const string&) const;
	const ServerSocket& operator >> (string&) const;

	void accept(ServerSocket&);

};


#endif
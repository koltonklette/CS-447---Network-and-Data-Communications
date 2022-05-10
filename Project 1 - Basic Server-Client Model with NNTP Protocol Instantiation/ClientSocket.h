//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//NOTE: Resource from Linux Gazette is cited in supplementary report.

#ifndef ClientSocket_class
#define ClientSocket_class

#include "Socket.h"
using namespace std;


class ClientSocket : private Socket
{
public:

	ClientSocket(string host, int port);
	virtual ~ClientSocket() {};

	const ClientSocket& operator << (const string&) const;
	const ClientSocket& operator >> (string&) const;

};


#endif